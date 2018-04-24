class ChatStatementImage
  include ActiveModel::Model

  attr_accessor :url
  attr_accessor :title
  attr_accessor :subtitle

  validates :url, presence: true, length: { maximum: 5000 }
  validates :title, length: { maximum: 5000 }
  validates :subtitle, length: { maximum: 5000 }
end
