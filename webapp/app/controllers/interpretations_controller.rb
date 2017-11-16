class InterpretationsController < ApplicationController
  before_action :set_agent
  before_action :check_user_rights
  before_action :set_intent_and_interpretation, except: [:create]

  def create
    @intent = @agent.intents.friendly.find(params[:intent_id])
    interpretation = Interpretation.new(interpretation_params)
    interpretation.intent = @intent
    @current_locale = interpretation.locale
    respond_to do |format|
      if interpretation.save
        format.js do
          @html_form = render_to_string(partial: 'form', locals: { intent: @intent, agent: @agent, interpretation: Interpretation.new, current_locale: @current_locale })
          @html = render_to_string(partial: 'interpretation', locals: { interpretation: interpretation })
          render partial: 'create_succeed'
        end
      else
        format.js do
          @html_form = render_to_string(partial: 'form', locals: { intent: @intent, agent: @agent, interpretation: interpretation, current_locale: @current_locale })
          render partial: 'create_failed'
        end
      end
    end
  end

  def show
    respond_to do |format|
      format.js {
        @show = render_to_string(partial: 'interpretation', locals: { interpretation: @interpretation })
        render partial: 'show'
      }
    end
  end

  def edit
    respond_to do |format|
      format.js {
        @form = render_to_string(partial: 'edit.html', locals: { interpretation: @interpretation })
        render partial: 'edit'
      }
    end
  end

  def update
    respond_to do |format|
      if @interpretation.update(interpretation_params)
        format.js {
          @show = render_to_string(partial: 'interpretation', locals: { interpretation: @interpretation })
          render partial: 'show'
        }
      else
        format.js do
          @form = render_to_string(partial: 'edit.html', locals: { interpretation: @interpretation })
          render partial: 'edit'
        end
      end
    end
  end

  def destroy
    respond_to do |format|
      if @interpretation.destroy
        format.js { render partial: 'destroy_succeed' }
      else
        format.js { render partial: 'destroy_failed' }
      end
    end
  end

  private

    def set_agent
      @agent = Agent.friendly.find(params[:agent_id])
    end

    def set_intent_and_interpretation
      @intent = @agent.intents.friendly.find(params[:intent_id])
      @interpretation = @intent.interpretations.find(params[:id])
    end

    def interpretation_params
      params.require(:interpretation).permit(:expression, :locale)
    end

    def check_user_rights
      case action_name
      when 'show'
        access_denied unless current_user.can? :show, @agent
      when 'create', 'edit', 'update', 'destroy'
        access_denied unless current_user.can? :edit, @agent
      else
        access_denied
      end
    end
end
