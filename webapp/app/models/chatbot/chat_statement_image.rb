class Chatbot::ChatStatementImage
  include ActiveModel::Model
  include Speechable

  attr_accessor :url
  attr_accessor :title
  attr_accessor :subtitle

  validates :url, presence: true, length: { maximum: 5000 }
  validates :title, length: { maximum: 100 }
  validates :subtitle, length: { maximum: 500 }

  def nature
    "image"
  end
end
