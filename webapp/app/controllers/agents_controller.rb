class AgentsController < ApplicationController
  before_action :set_user_and_agent, except: [:index, :new, :create]
  before_action :check_user_rights, except: [:index, :new, :create]

  def index
    @search = AgentSearch.new(current_user.id, search_params)
    @agents = Agent.search(@search.options).order(name: :asc)
                .page(params[:page]).per(12)
  end

  def show
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
        redirect_path = @origin == 'show' ? user_agent_path(current_user, @agent.agentname) : agents_path
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
    render partial: 'confirm_transfer_ownership', locals: { agent: @agent, errors: [] }
  end

  def transfer_ownership
    result = @agent.transfer_ownership_to(params[:users][:new_owner])

    respond_to do |format|
      if result[:success]
        format.json {
          new_owner = User.find(@agent.owner_id)
          redirect_to agents_path,
            notice: t(
              'views.agents.index.ownership_transferred',
              name: @agent.name, username: new_owner.username
            )
        }
      else
        format.json {
          render json: {
            html: render_to_string(
              partial: 'confirm_transfer_ownership',
              formats: :html,
              locals: { agent: @agent, errors: result[:errors] }
            ),
            status: 422
          }
        }
      end
    end
  end

  def search_users_for_transfer_ownership
    respond_to do |format|
      format.json {
        query = params[:q].strip
        @users = []
        unless query.nil?
          if query.size > 2
            @users = User.confirmed.where(
              "id != ? AND ( email LIKE ? OR username LIKE ? )",
              @agent.owner_id, "%#{query}%", "%#{query}%"
            ).limit(10)
          end
        end
      }
    end
  end

  def generate_token
    @agent.ensure_api_token
    render json: { api_token: @agent.api_token }
  end


  private

    def check_user_rights
      unless current_user.id == @user.id
        flash[:alert] = t('views.agents.index.authorisation_refused')
        redirect_to agents_path
      end
    end

    def set_user_and_agent
      @user = User.friendly.find(params[:user_id])
      @agent = @user.agents.friendly.find(params[:id])
    end

    def agent_params
      params.require(:agent).permit(
        :name, :agentname, :description, :color, :image, :remove_image, :api_token
      )
    end

    def search_params
      params.permit(search: [:query])[:search]
    end

end
