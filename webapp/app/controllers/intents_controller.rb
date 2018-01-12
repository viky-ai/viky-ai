class IntentsController < ApplicationController
  skip_before_action :verify_authenticity_token, only: [:update_positions]
  before_action :set_agent
  before_action :check_user_rights
  before_action :set_intent, except: [:new, :create, :confirm_destroy, :update_positions]

  def show
    @interpretation = Interpretation.new
    @interpretation.auto_solution_enabled = true
    @current_locale = params[:locale] || @intent.ordered_locales.first
  end

  def new
    @intent = Intent.new
    render partial: 'new'
  end

  def create
    @intent = Intent.new(intent_params)
    @intent.agent = @agent
    @intent.locales = ['*']
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
    visibility = params[:visibility] || 'is_public'
    intents_count = params[:ids].size
    params[:ids].each_with_index do |id, position|
      if Intent.where(agent_id: @agent.id, id: id).count == 1
        Intent.where(agent_id: @agent.id, id: id).first.update(position: intents_count - position - 1, visibility: visibility)
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

  def select_new_locale
    available_locales = Interpretation::LOCALES - @intent.locales
    render partial: 'select_new_locale', locals: { available_locales: available_locales }
  end


  def add_locale
    locale_to_add = params[:locale_to_add]
    @intent.locales << locale_to_add

    if @intent.save
      redirect_to user_agent_intent_path(@agent.owner, @agent, @intent, locale: locale_to_add)
    else
      redirect_to user_agent_intent_path(@agent.owner, @agent, @intent, locale: @intent.locales.first), alert: t(
          'views.intents.add_locale.errors_message',
          errors: @intent.errors.full_messages.join(', ')
      )
    end
  end

  def remove_locale
    locale_to_remove = params[:locale_to_remove]
    locale_to_remove_index = @intent.locales.index(locale_to_remove)
    previous_locale_index = locale_to_remove_index <= 0 ? 0 : locale_to_remove_index - 1
    @intent.locales -= [locale_to_remove]
    if @intent.save
      redirect_to user_agent_intent_path(@agent.owner, @agent, @intent, locale: @intent.locales[previous_locale_index])
    else
      redirect_to user_agent_intent_path(@agent.owner, @agent, @intent, locale: @intent.locales[locale_to_remove_index]), alert: t(
          'views.intents.remove_locale.errors_message',
          errors: @intent.errors.full_messages.join(', ')
      )
    end
  end

  private

    def intent_params
      params.require(:intent).permit(:intentname, :description, :visibility)
    end

    def set_agent
      @agent = Agent.friendly.find(params[:agent_id])
    end

    def set_intent
      intent_id = params[:intent_id]|| params[:id]
      @intent = @agent.intents.friendly.find(intent_id)
    end

    def check_user_rights
      case action_name
      when 'show'
        access_denied unless current_user.can? :show, @agent
      when 'new', 'create', 'edit', 'update', 'confirm_destroy', 'destroy', 'update_positions', 'select_new_locale', 'add_locale', 'remove_locale'
        access_denied unless current_user.can? :edit, @agent
      else
        access_denied
      end
    end
end
