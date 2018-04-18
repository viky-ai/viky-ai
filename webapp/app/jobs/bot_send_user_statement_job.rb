class BotSendUserStatementJob < ApplicationJob
  queue_as :bot

  def perform(*args)
    chat_session_id = args[0]
    text = args[1]

    ChatSession.find(chat_session_id).bot.send_user_statement(chat_session_id, text)
  end

end
