class ChatStatementButton
  include ActiveModel::Model
  include Speechable

  attr_accessor :text
  attr_accessor :payload

  validates :text, presence: true, length: { maximum: 2000 }
  validates :payload, presence: true
end
