class Intent < ApplicationRecord
  extend FriendlyId
  friendly_id :intentname, use: :history, slug_column: 'intentname'

  AVAILABLE_COLORS = %w(black red pink purple deep-purple indigo blue light-blue cyan teal green light-green lime yellow amber orange deep-orange brown)

  belongs_to :agent
  has_many :interpretations, dependent: :destroy

  serialize :locales, JSON

  validates :intentname, uniqueness: { scope: [:agent_id] }, length: { in: 3..25 }, presence: true
  validates :locales, presence: true
  validate :check_locales

  before_validation :clean_intentname
  before_create :set_position
  before_create :set_color

  after_save do
    if saved_change_to_attribute?(:intentname)
      Nlp::Package.new(self.agent).push
    end
  end

  after_destroy do
    Nlp::Package.new(self.agent).push
  end

  def interpretations_with_local(locale)
    interpretations.where(locale: locale)
  end

  def locales_by_usage
    locales.sort_by do |locale|
      interpretations.where(locale: locale).count * -1
    end
  end

  private

    def clean_intentname
      unless intentname.nil?
        self.intentname = intentname.parameterize(separator: '-')
      end
    end

    def check_locales
      if self.locales.present?
        for locale in self.locales
          if !Interpretation::Locales.include? locale
            errors.add(:locales, I18n.t('errors.intents.unknown_locale', current_locale: locale))
          end
        end
      end
    end

    def set_position
      unless agent.nil?
        if agent.intents.count == 0
          self.position = 0
        else
          self.position = agent.intents.maximum(:position) + 1
        end
      end
    end

    def set_color
      self.color = Intent::AVAILABLE_COLORS[Random.new.rand(0..Intent::AVAILABLE_COLORS.size-1)] if self.color.blank?
    end
end
