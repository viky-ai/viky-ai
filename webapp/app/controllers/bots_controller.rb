class BotsController < ApplicationController
  before_action :set_owner
  before_action :set_agent
  before_action :check_user_rights
  before_action :set_bot, except: [:index, :new, :create, :ping]
  skip_before_action :verify_authenticity_token, only: [:ping]

  def index
  end

  def new
    @bot = Bot.new
    render partial: 'new'
  end

  def create
    @bot = Bot.new(bot_params)
    @bot.agent = @agent
    respond_to do |format|
      if @bot.save
        format.json do
          redirect_to user_agent_bots_path(@agent.owner, @agent),
            notice: t('views.bots.new.success_message')
        end
      else
        format.json do
          render json: {
            replace_modal_content_with: render_to_string(partial: 'new', formats: :html)
          }, status: 422
        end
      end
    end
  end

  def edit
    render partial: 'edit'
  end

  def update
    respond_to do |format|
      if @bot.update(bot_params)
        format.json {
          redirect_to user_agent_bots_path(@owner, @agent),
            notice: t('views.bots.edit.success_message')
        }
      else
        format.json {
          render json: {
            replace_modal_content_with: render_to_string(partial: 'edit', formats: :html),
          }, status: 422
        }
      end
    end
  end

  def confirm_destroy
    render partial: 'confirm_destroy'
  end

  def destroy
    if @bot.destroy
      redirect_to user_agent_bots_path(@owner, @agent), notice: t(
        'views.bots.destroy.success_message', name: @bot.name
      )
    else
      redirect_to user_agent_bots_path(@owner, @agent), alert: t(
        'views.bots.destroy.errors_message',
        errors: @bot.errors.full_messages.join(', ')
      )
    end
  end

  def ping
    @endpoint = params[:endpoint]
    succeed, message = Bot.ping(@endpoint)
    render json: { succeed: succeed, message: message }
  end


  private

    def bot_params
      params.require(:bot).permit(:name, :endpoint, :wip_enabled)
    end

    def set_bot
      @bot = @agent.bots.find(params[:id])
    end

    def set_owner
      @owner = User.friendly.find(params[:user_id])
    end

    def set_agent
      @agent = @owner.agents.owned_by(@owner).friendly.find(params[:agent_id])
    end

    def check_user_rights
      case action_name
        when 'index'
          access_denied unless current_user.can? :show, @agent
        when 'new', 'create', 'edit', 'update', 'confirm_destroy', 'destroy', 'ping'
          access_denied unless current_user.can? :edit, @agent
        else
          access_denied
      end
    end
end
