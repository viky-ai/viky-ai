class ChatStatement < ApplicationRecord
  default_scope { order(created_at: :asc) }

  belongs_to :chat_session, touch: true

  enum speaker: [:user, :bot, :moderator]
  enum nature: [:text, :image, :button, :button_group, :card, :list, :notification]

  serialize :content, JSON

  validates :content, presence: true
  validate :content_format

  after_create :notify_bot
  after_create :notify_ui

  def to_html
    ApplicationController.renderer.render(
      partial: "/chat_statements/widget",
      locals: { statement: self }
    )
  end

  def component
    case nature
      when 'text'
        Chatbot::ChatStatementText.new(content)
      when 'image'
        Chatbot::ChatStatementImage.new(content)
      when 'button'
        Chatbot::ChatStatementButton.new(content)
      when 'button_group'
        Chatbot::ChatStatementButtonGroup.new(content)
      when 'card'
        Chatbot::ChatStatementCard.new(content)
      when 'list'
        Chatbot::ChatStatementList.new(content)
      when 'notification'
        Chatbot::ChatStatementNotification.new(content)
      else
        # Should be impossible
        raise ActiveRecord::RecordInvalid.new I18n.t('errors.chat_statement.invalid_nature', nature: nature)
    end
  end


  private

    def notify_bot
      if speaker == "user"
        BotSendUserStatementJob.perform_later(chat_session.id, "says", {
          text: content['text']
        })
      end
    end

    def notify_ui
      ActionCable.server.broadcast "chat_session_channel_#{chat_session.user.id}", {
        session_id: chat_session.id,
        action: "display_message",
        message: self.to_html
      }
    end

    def content_format
      return if content.blank?
      component = self.component
      if component.invalid?
        errors.add(:base, component.errors.full_messages.join(', '))
      end
    end
end

