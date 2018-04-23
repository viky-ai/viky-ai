class ChatStatementText
  include ActiveModel::Model

  attr_accessor :text

  validates :text, presence: true, length: { maximum: 5000 }

end
