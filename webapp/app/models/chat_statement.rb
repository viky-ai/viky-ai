class ChatStatement < ApplicationRecord
  belongs_to :chat_session

  default_scope { order(created_at: :asc) }

  enum speaker: [:user, :bot]
  enum nature: [:text]

  after_create :notify_bot


  private

    def notify_bot
      if speaker == "user"
        BotSendUserStatementJob.perform_later(chat_session.id, content)
      end
    end
end
