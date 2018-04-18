class ChatSession < ApplicationRecord
  belongs_to :user
  belongs_to :bot

  has_many :chat_statements, dependent: :destroy

  after_create :notify_bot


  private

    def notify_bot
      BotSendStartJob.perform_later(bot.id, id)
    end
end
