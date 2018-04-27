class BotSendUserStatementJob < ApplicationJob
  queue_as :bot

  rescue_from(StandardError) do |exception|
    ChatStatement.create(
      speaker: ChatStatement.speakers[:moderator],
      nature: ChatStatement.natures[:notification],
      content: { text: I18n.t('errors.bots.communication_failure') },
      chat_session: ChatSession.find(arguments[0])
    )
  end

  def perform(*args)
    chat_session_id = args[0]
    nature = args[1]
    content = args[2]

    chat_session = ChatSession.find(chat_session_id)
    case nature
    when 'text'
      chat_session.bot.send_user_text(chat_session_id, content)
    when 'button'
      chat_session.bot.send_user_payload(chat_session_id, content)
    else
      raise 'Unknown nature'
    end
  end

end
