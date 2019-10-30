class Interpretation < ApplicationRecord
  include Colorable
  include Positionable
  positionable_ancestor :agent
  include Movable

  extend FriendlyId
  friendly_id :interpretation_name, use: :history, slug_column: 'interpretation_name'

  belongs_to :agent, touch: true
  has_many :formulations, dependent: :destroy
  has_many :formulation_aliases, as: :formulation_aliasable, dependent: :destroy

  enum visibility: [:is_public, :is_private]

  validates :interpretation_name, uniqueness: { scope: [:agent_id] },
                         length: { in: 3..30 },
                         presence: true

  before_validation :clean_interpretation

  after_commit do
    agent.need_nlp_sync
  end

  def formulations_with_local(locale)
    formulations.where(locale: locale)
  end

  def slug
    "#{agent.slug}/interpretations/#{interpretation_name}"
  end

  def aliased_interpretations
    Interpretation.where(agent_id: agent_id)
          .joins(formulations: :formulation_aliases)
          .where(formulation_aliases: { formulation_aliasable: self })
          .distinct
          .order('position desc, created_at desc')
  end

  private

    def clean_interpretation
      return if interpretation_name.nil?
      self.interpretation_name = interpretation_name.parameterize(separator: '-')
    end
end
