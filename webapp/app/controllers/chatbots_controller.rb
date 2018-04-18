class ChatbotsController < ApplicationController
  before_action :set_bots

  def index
  end

  def show
    @bot = Bot.find(params[:id])
    unless ChatSession.where(user: current_user, bot: @bot).exists?
      @chat_session = ChatSession.new(user: current_user, bot: @bot)
      @chat_session.save
    else
      @chat_session = ChatSession.where(user: current_user, bot: @bot).last
    end
  end


  private

    def set_bots
      @bots = Bot.accessible_bots(current_user)
    end
end
