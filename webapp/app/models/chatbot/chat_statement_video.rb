class Chatbot::ChatStatementVideo
  include ActiveModel::Model
  include Speechable

  attr_accessor :params, :title, :subtitle

  validates :params, presence: true, length: { maximum: 5000 }
  validates :title, length: { maximum: 100 }
  validates :subtitle, length: { maximum: 500 }

  def nature
    "video"
  end

  def has_info?
    title.present? || subtitle.present?
  end
end
