class ChatSession < ApplicationRecord
  belongs_to :user
  belongs_to :bot

  has_many :chat_statements, dependent: :destroy
end
