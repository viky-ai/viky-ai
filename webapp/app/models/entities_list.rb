class EntitiesList < ApplicationRecord
  include Colorable
  include Positionable
  positionable_ancestor :agent
  include Movable

  extend FriendlyId
  friendly_id :listname, use: :history, slug_column: 'listname'

  belongs_to :agent, touch: true
  has_many :entities, dependent: :delete_all
  has_many :interpretation_aliases, as: :interpretation_aliasable, dependent: :destroy
  has_many :entities_imports, dependent: :destroy

  enum proximity: ExpressionProximity::ENTITY_PROXIMITES, _prefix: :proximity

  enum visibility: [:is_public, :is_private]

  validates :listname, uniqueness: { scope: [:agent_id] },
                       length: { in: 3..30 },
                       presence: true

  before_validation :clean_listname

  after_commit do
    agent.need_nlp_sync
  end

  def slug
    "#{agent.slug}/entities_lists/#{listname}"
  end

  def to_csv_in_io(io)
    io.write CSV.generate_line([
      I18n.t('activerecord.attributes.entity.terms'),
      I18n.t('activerecord.attributes.entity.auto_solution_enabled'),
      I18n.t('activerecord.attributes.entity.solution')
    ])

    # We need SQL pagination in order to minimise RAM usage.
    #
    # Paginate with a cursor intead of using Rails find_in_batches
    # Because we need an order sql clause.
    #
    cursor_max = entities.order("position DESC").first.position
    cursor_min = entities.order("position ASC").first.position
    cursor = cursor_max + 1
    while cursor > cursor_min do
      io.write(
        CSV.generate do |csv|
          entities.select(:terms, :auto_solution_enabled, :solution, :position)
            .where("position < ?", cursor)
            .order("position DESC")
            .limit(1_000)
            .each do |entity|
              terms = entity.terms.collect { |t|
                t['locale'] == Locales::ANY ? t['term'] : "#{t['term']}:#{t['locale']}"
              }.join('|')
              csv << [terms, entity.auto_solution_enabled, entity.solution]
            cursor = entity.position
          end
        end
      )
    end
  end

  def to_csv
    io = StringIO.new
    to_csv_in_io(io)
    io.string
  end

  def aliased_intents
    Intent.where(agent_id: agent_id)
          .joins(interpretations: :interpretation_aliases)
          .where(interpretation_aliases: { interpretation_aliasable: self })
          .distinct
          .order('position desc, created_at desc')
  end

  def proximity
    @proximity ||= ExpressionProximity.new(read_attribute(:proximity))
  end

  private

    def clean_listname
      return if listname.nil?
      self.listname = listname.parameterize(separator: '-')
    end
end
