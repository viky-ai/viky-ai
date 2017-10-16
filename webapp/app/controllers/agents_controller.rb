class AgentsController < ApplicationController
  before_action :set_agent, only: [:edit, :update, :confirm_destroy, :destroy, :confirm_transfer_ownership, :transfer_ownership]

  def index
    @search = AgentSearch.new(current_user.id, search_params)
    @agents = Agent.search(@search.options).order(name: :asc)
                .page(params[:page]).per(12)
  end

  def show
    @agent = Agent.friendly.find(params[:id])
  end

  def new
    @agent = Agent.new
    @origin = 'index'
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
        @origin = 'index'
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
    @origin = params[:origin]
    render partial: 'edit'
  end

  def update
    @origin = params[:origin]
    respond_to do |format|
      if @agent.update(agent_params)
        redirect_path = @origin == 'show' ? user_agent_path(current_user, params[:id]) : agents_path
        format.json{
          redirect_to redirect_path, notice: t('views.agents.edit.success_message')
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

  def confirm_transfer_ownership
    render partial: 'confirm_transfer_ownership', locals: { agent: @agent, users: User.where("id != ?", current_user.id) }
  end

  def transfer_ownership
    new_owner = User.friendly.find(params[:users][:new_owner])
    @agent.transfer_ownership_to(new_owner)
    @agent.save
    redirect_to agents_path, notice: t('views.agents.index.ownership_transferred', name: @agent.name, username: new_owner.username)
  end


  private

    def set_agent
      user = User.friendly.find(params[:user_id])
      @agent = user.agents.friendly.find(params[:id])
    end

    def agent_params
      params.require(:agent).permit(
        :name, :agentname, :description, :color, :image, :remove_image
      )
    end

    def search_params
      params.permit(search: [:query])[:search]
    end

end
