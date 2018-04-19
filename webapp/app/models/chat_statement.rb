class ChatStatement < ApplicationRecord
  default_scope { order(created_at: :asc) }

  belongs_to :chat_session

  enum speaker: [:user, :bot]
  enum nature: [:text]

  serialize :content, JSON

  validates :content, presence: true, length: { maximum: 5000 }
  validate :content_format

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
        BotSendUserStatementJob.perform_later(chat_session.id, content['text'])
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

    def content_format
      case nature
        when 'text'
          validator = ChatStatementTextValidator.new(content)
          errors.add(:content, validator.errors) if validator.invalid?
        else
          raise ActiveRecord::RecordInvalid.new I18n.t('errors.chat_statement.unknown_nature')
      end
    end
end
