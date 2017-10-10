class AgentsController < ApplicationController

  def index
    @agents = current_user.agents
    # TODO: Pagination
    # TODO: Recherche
  end

  def new
    @agent = Agent.new
  end

  def create
    @agent = Agent.new(agent_params)
    @agent.users << current_user
    if @agent.save
      redirect_to agents_path, notice: t('views.agents.new.success_message')
    else
      render :new
    end
  end


  private
    def agent_params
      params.require(:agent).permit(:name, :agentname)
    end

end
