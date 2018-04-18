class BotSendStartJob < ApplicationJob
  queue_as :bot

  def perform(*args)
    bot_id = args[0]
    chat_session_id = args[1]

    Bot.find(bot_id).send_start(chat_session_id)
  end

end
