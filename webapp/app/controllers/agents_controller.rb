class AgentsController < ApplicationController

  def index
    @agents = current_user.agents
    # TODO: Pagination
    # TODO: Recherche
  end

  def new
  end

  def create
    # @agent = Agent.new(agent_params)
    # @agent.users << current_user
    # if @agent.save
    #   redirect_to :index, notice: "it's ok"
    # else
    #   render :new
    # end
  end

end
