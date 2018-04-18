class ChatSession < ApplicationRecord
  belongs_to :user
  belongs_to :bot
end
