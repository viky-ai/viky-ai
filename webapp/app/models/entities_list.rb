class EntitiesList < ApplicationRecord
  include Colorable
  include Positionable
  positionable_class EntitiesList

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

  def update_entities_positions(entities)
    current_objs = Entity.where(entities_list_id: id, id: entities).order(position: :asc)
    Agent.no_touching do
      update_order(entities, current_objs)
    end
    touch
  end

  private

    def positionable_collection
      agent.entities_lists
    end

    def clean_listname
      return if listname.nil?
      self.listname = listname.parameterize(separator: '-')
    end

    def update_order(new_ids, current)
      count = current.count
      current.each do |item|
        new_position = new_ids.find_index(item.id)
        unless new_position.nil?
          item.record_timestamps = false
          item.update_attribute(:position, count - new_position - 1)
          item.record_timestamps = true
        end
      end
    end
end
