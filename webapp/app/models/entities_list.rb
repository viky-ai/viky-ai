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

  after_update_commit do
    agent.need_nlp_sync
    agent.touch unless agent.destroyed?
  end

  def slug
    "#{agent.slug}/entities_lists/#{listname}"
  end

  def to_csv_in_io(io)
    io.write CSV.generate_line([
      I18n.t('activerecord.attributes.entity.terms'),
      I18n.t('activerecord.attributes.entity.auto_solution_enabled'),
      I18n.t('activerecord.attributes.entity.solution'),
      I18n.t('activerecord.attributes.entity.case_sensitive'),
      I18n.t('activerecord.attributes.entity.accent_sensitive')
    ])
    io.write(
      CSV.generate do |csv|
        entities_in_ordered_batchs.each do |batch, _, _|
          batch.select(:terms, :auto_solution_enabled, :solution, :case_sensitive, :accent_sensitive, :position).each do |entity|
            terms = entity.terms.collect { |t|
              t['locale'] == Locales::ANY ? t['term'] : "#{t['term']}:#{t['locale']}"
            }.join('|')
            csv << [terms, entity.auto_solution_enabled, entity.solution, entity.case_sensitive, entity.accent_sensitive]
          end
        end
      end
    )
  end

  def to_csv
    io = StringIO.new
    to_csv_in_io(io)
    io.string
  end

  def aliased_intents
    Intent.where(agent_id: agent_id)
          .joins(formulations: :interpretation_aliases)
          .where(interpretation_aliases: { interpretation_aliasable: self })
          .distinct
          .order('position desc, created_at desc')
  end

  def proximity
    @proximity ||= ExpressionProximity.new(read_attribute(:proximity))
  end

  def entities_in_ordered_batchs(batch_size = 1_000)
    Enumerator.new do |block|
      if entities.exists?
        cursor_max, cursor_min = entities.pluck(Arel.sql 'MAX("entities"."position"), MIN("entities"."position")').first
        cursor = cursor_max
        loop do
          batch_max_position = cursor
          batch_min_position = (cursor - batch_size + 1).positive? ? cursor - batch_size + 1 : cursor_min
          if entities.where(position: batch_min_position..batch_max_position).exists?
            batch = entities.where(position: batch_min_position..batch_max_position).order(position: :desc)
            block << [batch, batch_max_position, batch_min_position]
          end
          cursor = (cursor - batch_size).positive? ? cursor - batch_size : cursor_min
          break if cursor <= cursor_min
        end
      else
        block << [entities.where('1 = 1'), 0, 0]
      end
    end
  end

  private

    def clean_listname
      return if listname.nil?
      self.listname = listname.parameterize(separator: '-')
    end
end
