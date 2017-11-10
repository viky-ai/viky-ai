class IntentsController < ApplicationController
  skip_before_action :verify_authenticity_token, only: [:update_positions]
  before_action :set_agent
  before_action :check_user_rights
  before_action :set_intent, except: [:new, :create, :confirm_destroy, :update_positions]

  def show
    @interpretation = Interpretation.new
  end

  def new
    @intent = Intent.new
    render partial: 'new'
  end

  def create
    @intent = Intent.new(intent_params)
    @intent.agent = @agent
    respond_to do |format|
      if @intent.save
        format.json do
          redirect_to user_agent_path(current_user, @agent), notice: t('views.intents.new.success_message')
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
      if @intent.update(intent_params)
        format.json {
          redirect_to user_agent_path(current_user, @agent), notice: t('views.intents.edit.success_message')
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

  def update_positions
    intents_count = params[:ids].size
    params[:ids].each_with_index do |id, position|
      if Intent.where(agent_id: @agent.id, id: id).count == 1
        Intent.where(agent_id: @agent.id, id: id).first.update(position: intents_count - position - 1)
      end
    end
  end

  def confirm_destroy
    @intent = @agent.intents.friendly.find(params[:intent_id])
    render partial: 'confirm_destroy', locals: { intent: @intent }
  end

  def destroy
    if @intent.destroy
      redirect_to user_agent_path(current_user, @agent), notice: t(
        'views.intents.destroy.success_message', name: @intent.intentname
      )
    else
      redirect_to user_agent_path(current_user, @agent), alert: t(
        'views.intents.destroy.errors_message',
        errors: @intent.errors.full_messages.join(', ')
      )
    end
  end


  private

    def intent_params
      params.require(:intent).permit(:intentname, :description)
    end

    def set_agent
      @agent = Agent.friendly.find(params[:agent_id])
    end

    def set_intent
      @intent = @agent.intents.friendly.find(params[:id])
    end

    def check_user_rights
      case action_name
      when 'show'
        access_denied unless current_user.can? :show, @agent
      when 'new', 'create', 'edit', 'update', 'confirm_destroy', 'destroy', 'update_positions'
        access_denied unless current_user.can? :edit, @agent
      else
        access_denied
      end
    end
end
