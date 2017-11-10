class InterpretationsController < ApplicationController
  before_action :check_user_rights


  def create
    interpretation = Interpretation.new(interpretation_params)
    intent = Intent.friendly.find(params[:intent_id])
    interpretation.intent = intent

    respond_to do |format|
      if interpretation.save
        format.js {
          @html_form = render_to_string(partial: 'form', locals: { intent: intent, agent: intent.agent, interpretation: Interpretation.new })
          @html = render_to_string(partial: 'interpretation', locals: { interpretation: interpretation })
          render partial: 'create_succeed'
        }
      else
        format.js {
          @html_form = render_to_string(partial: 'form', locals: { intent: intent, agent: intent.agent, interpretation: interpretation })
          render partial: 'create_failed'
        }
      end
    end

  end


  private

    def interpretation_params
      params.require(:interpretation).permit(:expression)
    end

    def check_user_rights
      agent = Agent.friendly.find(params[:agent_id])
      case action_name
        when 'create'
          access_denied unless current_user.can? :edit, agent
        else
          access_denied
      end
    end

end
