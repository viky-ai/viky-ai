class ChatStatementButton
  include ActiveModel::Model

  attr_accessor :text
  attr_accessor :payload

  validates :text, presence: true, length: { maximum: 2000 }
  validates :payload, presence: true

end
