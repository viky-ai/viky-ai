class AgentsController < ApplicationController
  before_action :set_owner_and_agent, except: [:index, :new, :create]
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
    @agent.memberships << Membership.new(user_id: current_user.id, rights: 'all')

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
    if @agent.destroy
      redirect_to agents_path, notice: t('views.agents.destroy.success_message', name: @agent.name)
    else
      redirect_to agents_path, alert: t(
        'views.agents.destroy.errors_message',
        errors: @agent.errors.full_messages.join(', ')
      )
    end
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
            @users = User.confirmed
              .where.not(id: @agent.owner_id)
              .where("email LIKE ? OR username LIKE ?", "%#{query}%", "%#{query}%")
              .limit(10)
          end
        end
      }
    end
  end


  private

    def check_user_rights
      case action_name
      when "show"
        access_denied unless current_user.can? :show, @agent
      when "edit", "update"
        access_denied unless current_user.can? :edit, @agent
      when "confirm_transfer_ownership", "transfer_ownership", "confirm_destroy", "destroy"
        access_denied unless current_user.owner?(@agent)
      end
    end

    def set_owner_and_agent
      @owner = User.friendly.find(params[:user_id])
      @agent = @owner.agents.friendly.find(params[:id])
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
