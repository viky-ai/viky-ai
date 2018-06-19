class BotSendStartJob < ApplicationJob
  queue_as :bot

  rescue_from(StandardError) do |exception|
    send_error_statement(arguments[1], I18n.t('errors.bots.unknown_request_failure'))
  end

  rescue_from(Errno::ECONNREFUSED) do |exception|
    send_error_statement(arguments[1], I18n.t('errors.bots.communication_failure'))
  end

  rescue_from(RestClient::RequestFailed) do |exception|
    send_error_statement(arguments[1], I18n.t('errors.bots.bot_failure'))
  end

  def perform(*args)
    bot_id = args[0]
    chat_session_id = args[1]
    user_id = args[2]

    user = User.find(user_id)
    Bot.find(bot_id).send_start(chat_session_id, user)
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
