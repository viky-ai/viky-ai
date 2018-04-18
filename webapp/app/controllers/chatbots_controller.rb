class ChatbotsController < ApplicationController
  before_action :set_bots

  def index
  end

  def show
    @bot = Bot.find(params[:id])
    unless ChatSession.where(user: current_user, bot: @bot).exists?
      @chat_session = ChatSession.new(user: current_user, bot: @bot)
      @chat_session.save
      chat_statement = ChatStatement.new
      chat_statement.chat_session = @chat_session
      chat_statement.speaker = :bot
      chat_statement.nature = :text
      chat_statement.content = "Hello"
      chat_statement.save
    else
      @chat_session = ChatSession.where(user: current_user, bot: @bot).last
    end
  end


  private

    def set_bots
      @bots = Bot.where(agent_id: current_user.agents.pluck(:id))
    end

end
