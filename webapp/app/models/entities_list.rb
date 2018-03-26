class EntitiesList < ApplicationRecord
  include Colorable
  include Positionable
  positionable_ancestor :agent

  extend FriendlyId
  friendly_id :listname, use: :history, slug_column: 'listname'

  belongs_to :agent, touch: true
  has_many :entities, dependent: :destroy
  has_many :interpretation_aliases, as: :interpretation_aliasable, dependent: :destroy

  enum visibility: [:is_public, :is_private]

  validates :listname, uniqueness: { scope: [:agent_id] },
                       length: { in: 3..30 },
                       presence: true

  before_validation :clean_listname

  def slug
    "#{agent.slug}/entities_lists/#{listname}"
  end

  def to_csv
    options = {
      headers: [
        I18n.t('activerecord.attributes.entity.terms'),
        I18n.t('activerecord.attributes.entity.auto_solution_enabled'),
        I18n.t('activerecord.attributes.entity.solution')
      ],
      write_headers: true
    }
    CSV.generate(options) do |csv|
      entities.order(position: :desc).each do |entity|
        terms = entity.terms
                      .collect { |t| t['locale'] == Locales::ANY ? t['term'] : "#{t['term']}:#{t['locale']}" }
                      .join('|')
        csv << [terms, entity.auto_solution_enabled, entity.solution]
      end
    end
  end

  def from_csv(entities_import)
    entities_import.proceed(self)
  end

  def available_destinations(current_user)
    current_agent_id = agent.id
    current_user.agents
      .where(memberships: { rights: [:all, :edit] })
      .where.not(id: current_agent_id)
  end


  def change_agent(current_user, agent_destination)
    if agent_destination.nil?
      errors.add(:agent, I18n.t('errors.entities_lists.unknown_agent'))
      return false
    end
    unless current_user.can?(:edit, agent_destination)
      errors.add(:agent, I18n.t('errors.entities_lists.not_editable_agent'))
      return false
    end
    ActiveRecord::Base.transaction do
      self.agent = agent_destination
      self.position = agent_destination.entities_lists.present? ? agent_destination.entities_lists.maximum(:position) + 1 : 0
      save
    end
  end

  private

    def clean_listname
      return if listname.nil?
      self.listname = listname.parameterize(separator: '-')
    end
end
