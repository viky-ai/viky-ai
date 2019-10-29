class FormulationsController < ApplicationController
  skip_before_action :verify_authenticity_token, only: [:update_positions, :update_locale]
  before_action :set_owner
  before_action :set_agent
  before_action :check_user_rights
  before_action :set_intent
  before_action :set_formulation, except: [:create, :update_positions]

  def create
    formulation = Formulation.new(formulation_params)
    formulation.intent = @intent
    @current_locale = formulation.locale
    respond_to do |format|
      if formulation.save
        format.js do
          new_formulation = Formulation.new(
            proximity: 'close',
            keep_order: true,
            auto_solution_enabled: true
          )
          @html_form = render_to_string(partial: 'form', locals: {
            intent: @intent,
            agent: @agent,
            formulation: new_formulation,
            current_locale: @current_locale }
          )
          @html = render_to_string(partial: 'formulation', locals: {
            formulation: formulation,
            can_edit: current_user.can?(:edit, @agent),
            intent: @intent,
            agent: @agent,
            owner: @owner
          })
          render partial: 'create_succeed'
        end
      else
        format.js do
          @html_form = render_to_string(partial: 'form', locals: {
            intent: @intent,
            agent: @agent,
            formulation: formulation,
            current_locale: @current_locale }
          )
          render partial: 'create_failed'
        end
      end
    end
  end

  def show
    respond_to do |format|
      format.js {
        @show = render_to_string(partial: 'formulation', locals: {
          formulation: @formulation,
          can_edit: current_user.can?(:edit, @agent),
          intent: @intent,
          agent: @agent,
          owner: @owner
        })
        render partial: 'show'
      }
    end
  end

  def show_detailed
    respond_to do |format|
      format.js {
        @show = render_to_string(partial: 'show_detailed.html', locals: { formulation: @formulation })
        render partial: 'show_detailed'
      }
    end
  end

  def edit
    respond_to do |format|
      format.js {
        @form = render_to_string(partial: 'edit.html', locals: { formulation: @formulation })
        render partial: 'edit'
      }
    end
  end

  def update
    respond_to do |format|
      if @formulation.update(formulation_params)
        format.js {
          @show = render_to_string(partial: 'formulation', locals: {
            formulation: @formulation,
            can_edit: current_user.can?(:edit, @agent),
            intent: @intent,
            agent: @agent,
            owner: @owner
          })
          render partial: 'show'
        }
      else
        format.js do
          @form = render_to_string(partial: 'edit.html', locals: { formulation: @formulation })
          render partial: 'edit'
        end
      end
    end
  end

  def destroy
    respond_to do |format|
      if @formulation.destroy
        format.js { render partial: 'destroy_succeed' }
      else
        format.js { render partial: 'destroy_failed' }
      end
    end
  end

  def update_positions
    Formulation.update_positions(@intent, params[:ids])
  end

  def update_locale
    previous_locale = @formulation.update_locale params[:locale]
    redirect_to user_agent_intent_path(@owner, @agent, @intent, { locale: previous_locale })
  end


  private

    def set_owner
      @owner = User.friendly.find(params[:user_id])
    end

    def set_agent
      @agent = Agent.owned_by(@owner).friendly.find(params[:agent_id])
    end

    def set_intent
      @intent = @agent.intents.friendly.find(params[:intent_id])
    end

    def set_formulation
      @formulation = @intent.formulations.find(params[:id])
    end

    def formulation_params
      p = params.require(:formulation).permit(
        :expression, :locale, :keep_order, :proximity, :solution, :auto_solution_enabled,
        :formulation_aliases_attributes => [
          :id, :nature, :position_start, :position_end,
          :aliasname, :reg_exp, :formulation_aliasable_id, :formulation_aliasable_type,
          :is_list, :any_enabled,
          :_destroy
        ]
      )
      unless p[:formulation_aliases_attributes].nil?
        for item in p[:formulation_aliases_attributes]
          item[:is_list]     = false if item[:is_list].blank?
          item[:any_enabled] = false if item[:any_enabled].blank?
        end
      end
      p
    end

    def check_user_rights
      case action_name
      when 'show', 'show_detailed'
        access_denied unless current_user.can? :show, @agent
      when 'create', 'edit', 'update', 'destroy', 'update_positions', 'update_locale'
        access_denied unless current_user.can? :edit, @agent
      else
        access_denied
      end
    end
end
