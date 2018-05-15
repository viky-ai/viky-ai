class Chatbot::ChatStatementVideo
  include ActiveModel::Model
  include Speechable

  attr_accessor :params

  validates :params, presence: true, length: { maximum: 5000 }

  def nature
    "video"
  end
end
