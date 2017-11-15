class Intent < ApplicationRecord
  extend FriendlyId
  friendly_id :intentname, use: :history, slug_column: 'intentname'

  belongs_to :agent
  has_many :interpretations, dependent: :destroy

  validates :intentname, uniqueness: { scope: [:agent_id] }, length: { in: 3..25 }, presence: true

  before_validation :clean_intentname
  before_create :set_position

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

  private

    def clean_intentname
      unless intentname.nil?
        self.intentname = intentname.parameterize(separator: '-')
      end
    end

    def set_position
      unless agent.nil?
        self.position = agent.intents.count
      end
    end
end
