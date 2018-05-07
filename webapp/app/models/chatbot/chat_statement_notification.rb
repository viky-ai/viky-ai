class Chatbot::ChatStatementNotification
  include ActiveModel::Model

  attr_accessor :text

  validates :text, presence: true, length: { maximum: 2000 }

  def nature
    "notification"
  end
end
