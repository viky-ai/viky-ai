class Chatbot::ChatStatementVideo
  include ActiveModel::Model
  include Speechable

  attr_accessor :params, :title, :description

  validates :params, presence: true, length: { maximum: 5000 }
  validates :title, length: { maximum: 100 }
  validates :description, length: { maximum: 500 }

  def nature
    "video"
  end

  def has_info?
    title.present? || description.present?
  end
end
