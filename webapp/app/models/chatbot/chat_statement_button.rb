class Chatbot::ChatStatementButton
  include ActiveModel::Model
  include Speechable

  attr_accessor :text, :payload, :disabled

  validates :text, presence: true, length: { maximum: 2000 }
  validates :disabled, inclusion: { in: [true, false] }, allow_nil: true
  validates :payload, presence: true

  def is_disabled?
    self.disabled == true
  end
end
