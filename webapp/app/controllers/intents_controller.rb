class IntentsController < ApplicationController
  skip_before_action :verify_authenticity_token, only: [:update_positions]
  before_action :set_owner
  before_action :set_agent
  before_action :check_user_rights
  before_action :set_intent, except: [:index, :new, :create, :confirm_destroy, :update_positions]

  def index
    @intents = @agent.intents.includes(:interpretations).order('position desc, created_at desc')
    ui_state = UserUiState.new current_user
    @last_agent = ui_state.last_destination_agent(@agent)
  end

  def show
    @interpretation = Interpretation.new
    @interpretation.glued = true
    @interpretation.keep_order = true
    @interpretation.auto_solution_enabled = true
    @current_locale = params[:locale] || @intent.ordered_locales.first
  end

  def new
    @intent = Intent.new(visibility: Intent.visibilities.key(Intent.visibilities[:is_private]))
    render partial: 'new'
  end

  def create
    @intent = Intent.new(intent_params)
    @intent.agent = @agent
    @intent.locales = [Locales::ANY]
    respond_to do |format|
      if @intent.save
        format.json do
          redirect_to user_agent_intents_path(@agent.owner, @agent),
            notice: t('views.intents.new.success_message')
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
          redirect_to user_agent_intents_path(@owner, @agent),
            notice: t('views.intents.edit.success_message')
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
    Intent.update_positions(@agent, params[:is_public], params[:is_private])
  end

  def confirm_destroy
    @intent = @agent.intents.friendly.find(params[:intent_id])
    render partial: 'confirm_destroy', locals: { intent: @intent }
  end

  def destroy
    if @intent.destroy
      redirect_to user_agent_intents_path(@owner, @agent), notice: t(
        'views.intents.destroy.success_message', name: @intent.intentname
      )
    else
      redirect_to user_agent_intents_path(@owner, @agent), alert: t(
        'views.intents.destroy.errors_message',
        errors: @intent.errors.full_messages.join(', ')
      )
    end
  end

  def select_new_locale
    available_locales = Locales::ALL - @intent.locales
    render partial: 'select_new_locale', locals: { available_locales: available_locales }
  end

  def add_locale
    locale_to_add = params[:locale_to_add]
    @intent.locales << locale_to_add

    if @intent.save
      redirect_to user_agent_intent_path(@owner, @agent, @intent, locale: locale_to_add)
    else
      redirect_to user_agent_intent_path(@owner, @agent, @intent, locale: @intent.locales.first), alert: t(
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
      redirect_to user_agent_intent_path(@owner, @agent, @intent, locale: @intent.locales[previous_locale_index])
    else
      redirect_to user_agent_intent_path(@owner, @agent, @intent, locale: @intent.locales[locale_to_remove_index]), alert: t(
          'views.intents.remove_locale.errors_message',
          errors: @intent.errors.full_messages.join(', ')
      )
    end
  end

  def move_to_agent
    ui_state = UserUiState.new current_user
    if @intent.move_to_agent(@agent_destination)
      ui_state.last_destination_agent = @agent_destination.id
      ui_state.save
      redirect_to user_agent_intents_path(@owner, @agent), notice: {
        i18n_key: 'views.intents.move_to.success_message_html',
        locals: {
          name: @intent.intentname,
          agent_name: @agent_destination.name,
          agent_link: user_agent_intents_path(@agent_destination.owner, @agent_destination)
        }
      }
    else
      redirect_to user_agent_intents_path(@owner, @agent), alert: t(
        'views.intents.move_to.errors_message',
        errors: @intent.errors.full_messages.join(', ')
      )
    end
  end


  private

    def intent_params
      params.require(:intent).permit(:intentname, :description, :visibility)
    end

    def set_owner
      @owner = User.friendly.find(params[:user_id])
    end

    def set_agent
      @agent = @owner.agents.friendly.find(params[:agent_id])
    end

    def set_intent
      intent_id = params[:intent_id]|| params[:id]
      @intent = @agent.intents.friendly.find(intent_id)
    end

    def check_user_rights
      case action_name
      when 'show', 'index'
        access_denied unless current_user.can? :show, @agent
      when 'new', 'create', 'edit', 'update', 'confirm_destroy',
           'destroy', 'update_positions', 'select_new_locale',
           'add_locale', 'remove_locale'
        access_denied unless current_user.can? :edit, @agent
      when 'move_to_agent'
        if current_user.can? :edit, @agent
          user_destination = User.friendly.find(params[:user])
          @agent_destination = user_destination.agents.friendly.find(params[:agent])
          access_denied unless current_user.can? :edit, @agent_destination
        else
          access_denied
        end
      else
        access_denied
      end
    end
end
