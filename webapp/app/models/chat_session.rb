class ChatSession < ApplicationRecord
  default_scope { order(created_at: :asc) }

  belongs_to :user
  belongs_to :bot
  has_many :chat_statements, dependent: :destroy

  after_create :notify_bot

  def expired?
    last_session = ChatSession.where(user: user, bot: bot).last
    id != last_session.id
  end

  private

    def notify_bot
      BotSendStartJob.set(wait: 0.25).perform_later(bot.id, id)
    end
end
