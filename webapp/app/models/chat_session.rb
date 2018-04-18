class ChatSession < ApplicationRecord
  default_scope { order(created_at: :asc) }

  belongs_to :user
  belongs_to :bot
  has_many :chat_statements, dependent: :destroy

  after_create :notify_bot


  private

    def notify_bot
      BotSendStartJob.set(wait: 0.2).perform_later(bot.id, id)
    end
end
