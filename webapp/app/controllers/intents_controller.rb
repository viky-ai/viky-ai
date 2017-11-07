class IntentsController < ApplicationController

  def new
    @agent = Agent.friendly.find(params[:agent_id])
    @intent = Intent.new
    render partial: 'new'
  end

  def create
    @intent = Intent.new(intent_params)
    @agent = current_user.agents.friendly.find(params[:agent_id])
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

  private

  def intent_params
    params.require(:intent).permit(:intentname, :description)
  end
end
