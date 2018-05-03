class BotSendUserStatementJob < ApplicationJob
  queue_as :bot

  rescue_from(StandardError) do |exception|
    send_error_statement(arguments[0], I18n.t('errors.bots.unknown_request_failure'))
  end

  rescue_from(Errno::ECONNREFUSED) do |exception|
    send_error_statement(arguments[0], I18n.t('errors.bots.communication_failure'))
  end

  rescue_from(RestClient::RequestFailed) do |exception|
    send_error_statement(arguments[0], I18n.t('errors.bots.bot_failure'))
  end

  def perform(*args)
    chat_session_id = args[0]
    nature          = args[1]
    content         = args[2]

    chat_session = ChatSession.find(chat_session_id)
    case nature
    when 'text'
      chat_session.bot.send_user_text(chat_session_id, content)
    when 'button'
      chat_session.bot.send_user_payload(chat_session_id, content)
    else
      # Should be impossible
      raise I18n.t('errors.chat_statement.invalid_nature')
    end
  end

  private
    def send_error_statement(session_id, message)
      ChatStatement.create(
        speaker: ChatStatement.speakers[:moderator],
        nature: ChatStatement.natures[:notification],
        content: { text: message },
        chat_session: ChatSession.find(session_id)
      )
    end
end
