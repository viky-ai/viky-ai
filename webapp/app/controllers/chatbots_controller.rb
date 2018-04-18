class ChatbotsController < ApplicationController
  before_action :set_bots

  def index
  end

  def show
    @bot = Bot.find(params[:id])

    if ChatSession.where(user: current_user, bot: @bot).exists?
      @chat_session = ChatSession.where(user: current_user, bot: @bot).last
    else
      @chat_session = ChatSession.new(user: current_user, bot: @bot)
      @chat_session.save
    end

  end

  def reset
    @bot = Bot.find(params[:id])
    ChatSession.new(user: current_user, bot: @bot).save

    redirect_to chatbot_path(@bot)
  end


  private

    def set_bots
      @bots = Bot.accessible_bots(current_user)
    end
end
