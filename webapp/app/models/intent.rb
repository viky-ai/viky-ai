class Intent < ApplicationRecord
  include Colorable
  include Positionable
  positionable_ancestor :agent
  include Movable

  extend FriendlyId
  friendly_id :intentname, use: :history, slug_column: 'intentname'

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

  after_commit do
    agent.need_nlp_sync
  end

  def interpretations_with_local(locale)
    interpretations.where(locale: locale)
  end

  def ordered_locales
    Locales::ALL.select { |l| locales.include?(l) }
  end

  def slug
    "#{agent.slug}/interpretations/#{intentname}"
  end

  def aliased_intents
    Intent.where(agent_id: agent_id)
          .joins(interpretations: :interpretation_aliases)
          .where(interpretation_aliases: { interpretation_aliasable: self })
          .distinct
          .order('position desc, created_at desc')
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
end
