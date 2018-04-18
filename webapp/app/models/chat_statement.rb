class ChatStatement < ApplicationRecord
  default_scope { order(created_at: :asc) }

  belongs_to :chat_session

  enum speaker: [:user, :bot]
  enum nature: [:text]

  after_create :notify_bot
  after_create :notify_user

  private

    def notify_bot
      if speaker == "user"
        BotSendUserStatementJob.perform_later(chat_session.id, content)
      end
    end

  def notify_user
    if speaker == "bot"
      message = ApplicationController.renderer.render(partial: '/chat_statements/text', locals: { statement: self })
      ActionCable.server.broadcast 'chat_session_channel', { session_id: chat_session.id, message: message }
    end
  end
end
