class ChatSession < ApplicationRecord
  default_scope { order(created_at: :asc) }

  belongs_to :user
  belongs_to :bot
  has_many :chat_statements, dependent: :destroy

  validates :locale, inclusion: { in: lambda { |object| ChatSession.locales.keys.map(&:to_s) } }

  after_create :notify_bot
  after_update :notify_ui_if_locale_changed

  def expired?
    last_session = ChatSession.where(user: user, bot: bot).last
    id != last_session.id
  end

  def self.locales
    {
      "ru-RU": { # Russe
        language: "Pусский",
        country: "Default"
      },
      "ar": {    # Arabe
        language: "العربية",
        country: "Default"
      },
      "ja-JP": { # Japonais
        language: "日本語",
        country: "Default"
      },
      "ko-KR": { # Coréen
        language: "한국어",
        country: "Default"
      },
      "zh": {    # Chinois
        language: "中国",
        country: "Default"
      },
      "en-US": {
        language: "English",
        country: "United States"
      },
      "en-GB": {
        language: "English",
        country: "United Kingdom"
      },
      "fr-FR": {
        language: "Français",
        country: "Default"
      },
      "es-ES": {
        language: "Español",
        country: "España"
      },
      "it-IT": {
        language: "Italiano",
        country: "Italia"
      },
      "de-DE": {
        language: "Deutsch",
        country: "Default"
      }
    }
  end


  private

    def notify_bot
      BotSendStartJob.set(wait: 0.25).perform_later(bot.id, id)
    end

    def notify_ui_if_locale_changed
      if self.saved_change_to_attribute?(:locale)
        ActionCable.server.broadcast "chat_session_channel_#{self.user.id}", {
          session_id: self.id,
          action: "update_locale",
          locale: self.locale
        }
      end
    end
end
