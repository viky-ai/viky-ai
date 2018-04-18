class ChatStatement < ApplicationRecord
  belongs_to :chat_session

  enum speaker: [:user, :bot]
  enum nature: [:text]
end
