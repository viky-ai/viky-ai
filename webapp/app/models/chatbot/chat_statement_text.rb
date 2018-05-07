class Chatbot::ChatStatementText
  include ActiveModel::Model
  include Speechable

  attr_accessor :text

  validates :text, presence: true, length: { maximum: 5000 }

  def nature
    "text"
  end
end
