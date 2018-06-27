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

  def perform(chat_session_id, user_action, data)
    chat_session = ChatSession.find(chat_session_id)
    case user_action
    when 'says'
      chat_session.bot.send_user_text(chat_session_id, data[:text])
    when 'click_button'
      statement = ChatStatement.find(data[:statement_id])
      if statement.button_group?
        if statement.component.disable_on_click?
          statement.component.disable(statement, data[:button_index])
        end
      end
      chat_session.bot.send_user_payload(chat_session_id, data[:payload])
    when 'locate'
      chat_session.bot.send_user_location(chat_session_id, data[:status], data[:location])
    else
      # Should be impossible
      raise I18n.t('errors.chat_statement.invalid_nature', nature: user_action)
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
