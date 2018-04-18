class ChatbotsController < ApplicationController
  before_action :set_bots

  def index
  end

  def show
    @bot = Bot.find(params[:id])
  end


  private

    def set_bots
      @bots = Bot.where(agent_id: current_user.agents.pluck(:id))
    end

end
