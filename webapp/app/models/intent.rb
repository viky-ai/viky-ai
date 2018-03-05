class Intent < ApplicationRecord
  extend FriendlyId
  friendly_id :intentname, use: :history, slug_column: 'intentname'

  AVAILABLE_COLORS = %w[black red pink purple deep-purple indigo blue
                        light-blue cyan teal green light-green lime
                        yellow amber orange deep-orange brown].freeze

  belongs_to :agent, touch: true
  has_many :interpretations, dependent: :destroy
  has_many :interpretation_aliases, as: :interpretation_aliasable, dependent: :destroy

  serialize :locales, JSON

  enum visibility: [:is_public, :is_private]

  validates :intentname, uniqueness: { scope: [:agent_id] },
                         length: { in: 3..30 },
                         presence: true
  validates :locales, presence: true
  validate :check_locales

  before_validation :clean_intentname
  before_create :set_position
  before_create :set_color

  before_destroy :update_dependent_agents, prepend: true

  def interpretations_with_local(locale)
    interpretations.where(locale: locale)
  end

  def ordered_locales
    Locales::ALL.select { |l| locales.include?(l) }
  end

  def slug
    "#{agent.slug}/interpretations/#{intentname}"
  end


  private

    def clean_intentname
      return if intentname.nil?
      self.intentname = intentname.parameterize(separator: '-')
    end

    def check_locales
      return if locales.blank?
      locales.each do |locale|
        unless Locales::ALL.include? locale
          errors.add(:locales, I18n.t('errors.intents.unknown_locale', current_locale: locale))
        end
      end
    end

    def set_position
      return if agent.nil?
      if agent.intents.count.zero?
        self.position = 0
      else
        self.position = agent.intents.maximum(:position) + 1
      end
    end

    def set_color
      return if color.present?
      random_index = Random.new.rand(0..Intent::AVAILABLE_COLORS.size - 1)
      self.color = Intent::AVAILABLE_COLORS[random_index]
    end

    def update_dependent_agents
      InterpretationAlias.where(intent_id: id).each do |ialias|
        ialias.interpretation.intent.touch
      end
    end
end
