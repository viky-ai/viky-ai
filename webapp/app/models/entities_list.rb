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

  def slug
    "#{agent.slug}/#{listname}"
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
end
