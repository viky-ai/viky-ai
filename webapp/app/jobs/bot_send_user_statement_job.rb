class BotSendUserStatementJob < ApplicationJob
  queue_as :bot

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
