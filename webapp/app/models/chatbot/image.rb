class Chatbot::Image
  include ActiveModel::Model
  include Speechable

  attr_accessor :url, :title, :description

  validates :url, presence: true, length: { maximum: 5000 }
  validates :title, length: { maximum: 100 }
  validates :description, length: { maximum: 500 }

  def nature
    "image"
  end

  def has_info?
    title.present? || description.present?
  end
end
