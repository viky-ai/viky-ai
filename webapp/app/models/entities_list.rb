class EntitiesList < ApplicationRecord
  extend FriendlyId
  friendly_id :listname, use: :history, slug_column: 'listname'

  AVAILABLE_COLORS = %w[black red pink purple deep-purple indigo blue
                        light-blue cyan teal green light-green lime
                        yellow amber orange deep-orange brown].freeze

  belongs_to :agent, touch: true
  has_many :entities
  has_many :interpretation_aliases, as: :interpretation_aliasable, dependent: :destroy

  enum visibility: [:is_public, :is_private]

  validates :listname, uniqueness: { scope: [:agent_id] },
                       length: { in: 3..30 },
                       presence: true

  before_validation :clean_listname
  before_create :set_color
  before_create :set_position

  def slug
    "#{agent.slug}/entities_lists/#{listname}"
  end

  def to_csv
    options = {
      col_sep: ',',
      quote_char: "'",
      force_quotes: true,
      headers: [I18n.t('activerecord.attributes.entity.terms'), I18n.t('activerecord.attributes.entity.auto_solution_enabled'), I18n.t('activerecord.attributes.entity.solution')],
      write_headers: true
    }
    CSV.generate(options) do |csv|
      entities.each do |entity|
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

  private

    def clean_listname
      return if listname.nil?
      self.listname = listname.parameterize(separator: '-')
    end

    def set_color
      return if color.present?
      random_index = Random.new.rand(0..Intent::AVAILABLE_COLORS.size - 1)
      self.color = Intent::AVAILABLE_COLORS[random_index]
    end

    def set_position
      return if agent.nil?
      if self.position.zero?
        self.position = agent.entities_lists.count.zero? ? 0 : agent.entities_lists.maximum(:position) + 1
      end
    end
end
