class ChatStatement < ApplicationRecord
  default_scope { order(created_at: :asc) }

  belongs_to :chat_session, touch: true

  enum speaker: [:user, :bot, :moderator]
  enum nature: [:text, :image, :video, :map, :button, :button_group, :card, :list, :notification, :geolocation]

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
        Chatbot::Text.new(content)
      when 'image'
        Chatbot::Image.new(content)
      when 'video'
        Chatbot::Video.new(content)
      when 'map'
        Chatbot::Map.new(content)
      when 'button'
        Chatbot::Button.new(content)
      when 'button_group'
        Chatbot::ButtonGroup.new(content)
      when 'card'
        Chatbot::Card.new(content)
      when 'list'
        Chatbot::List.new(content)
      when 'notification'
        Chatbot::Notification.new(content)
      when 'geolocation'
        Chatbot::Geolocation.new(content)
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

