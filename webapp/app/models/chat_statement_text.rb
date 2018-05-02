class ChatStatementText
  include ActiveModel::Model
  include Speechable

  attr_accessor :text

  validates :text, presence: true, length: { maximum: 5000 }
end
