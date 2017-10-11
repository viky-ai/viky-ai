class AgentsController < ApplicationController
  before_action :set_agent, only: [:edit, :update, :confirm_destroy, :destroy]

  def index
    @agents = current_user.agents
    @search = AgentSearch.new
    # TODO: Pagination
    # TODO: Recherche
  end

  def new
    @agent = Agent.new
    render partial: 'new'
  end

  def create
    @agent = Agent.new(agent_params)
    @agent.users << current_user

    respond_to do |format|
      if @agent.save
        format.json{
          redirect_to agents_path, notice: t('views.agents.new.success_message')
        }
      else
        format.json{
          render json: {
            html: render_to_string(partial: 'new', formats: :html),
            status: 422
          }
        }
      end
    end
  end

  def edit
    render partial: 'edit'
  end

  def update
    respond_to do |format|
      if @agent.update(agent_params)
        format.json{
          redirect_to agents_path, notice: t('views.agents.edit.success_message')
        }
      else
        format.json{
          render json: {
            html: render_to_string(partial: 'edit', formats: :html),
            status: 422
          }
        }
      end
    end
  end

  def confirm_destroy
    render partial: 'confirm_destroy', locals: { agent: @agent }
  end

  def destroy
    @agent.destroy
    redirect_to agents_path, notice: t('views.agents.destroy.message', name: @agent.name)
  end


  private

    def set_agent
      user = User.friendly.find(params[:user_id])
      @agent = user.agents.friendly.find(params[:id])
    end

    def agent_params
      params.require(:agent).permit(:name, :agentname)
    end

end
