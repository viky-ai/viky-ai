class SuccessorsController < ApplicationController
  before_action :set_owner_and_agent

  def new
    @available_successors = @agent.available_successors(current_user)
    render partial: 'new'
  end

  def create
    arc = AgentArc.new(source: @agent, target_id: params[:id])
    if arc.save
      redirect_to user_agent_path(@agent.owner, @agent.agentname)
    else
      redirect_to user_agent_path(@agent.owner, @agent.agentname), alert: arc.errors.full_messages.join(', ')
    end
  end


  private

    def set_owner_and_agent
      begin
        @owner = User.friendly.find(params[:user_id])
        @agent = @owner.agents.friendly.find(params[:agent_id])
      rescue ActiveRecord::RecordNotFound
        redirect_to '/404'
      end
    end

end
