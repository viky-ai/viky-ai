class BotSendStartJob < ApplicationJob
  queue_as :bot

  rescue_from(StandardError) do |exception|
    ChatStatement.create(
      speaker: ChatStatement.speakers[:moderator],
      nature: ChatStatement.natures[:notification],
      content: { text: I18n.t('errors.bots.communication_failure') },
      chat_session: ChatSession.find(arguments[1])
    )
  end

  def perform(*args)
    bot_id = args[0]
    chat_session_id = args[1]

    Bot.find(bot_id).send_start(chat_session_id)
  end

end
