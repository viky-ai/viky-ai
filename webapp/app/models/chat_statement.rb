class ChatStatement < ApplicationRecord
  default_scope { order(created_at: :asc) }

  belongs_to :chat_session

  enum speaker: [:user, :bot]
  enum nature: [:text]

  validates :content, presence: true

  after_create :notify_bot
  after_create :notify_user

  def to_html
    ApplicationController.renderer.render(
      partial: "/chat_statements/#{self.nature}",
      locals: { statement: self }
    )
  end

  private

    def notify_bot
      if speaker == "user"
        BotSendUserStatementJob.perform_later(chat_session.id, content)
      end
    end

    def notify_user
      if speaker == "bot"
        ActionCable.server.broadcast "chat_session_channel_#{chat_session.user.id}", {
          session_id: chat_session.id,
          message: self.to_html
        }
      end
    end
end
