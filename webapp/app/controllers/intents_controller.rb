class IntentsController < ApplicationController
  skip_before_action :verify_authenticity_token, only: [:update_positions]

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

  def edit
    @agent = Agent.friendly.find(params[:agent_id])
    @intent = @agent.intents.friendly.find(params[:id])
    render partial: 'edit'
  end

  def update
    @agent = Agent.friendly.find(params[:agent_id])
    @intent = @agent.intents.friendly.find(params[:id])

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
    @agent = Agent.friendly.find(params[:agent_id])
    intents_count = params[:ids].size
    params[:ids].each_with_index do |id, position|
      if Intent.where(agent_id: @agent.id, id: id).count == 1
        Intent.where(agent_id: @agent.id, id: id).first.update(position: intents_count - position - 1)
      end
    end
  end

  def confirm_destroy
    @agent = Agent.friendly.find(params[:agent_id])
    @intent = @agent.intents.friendly.find(params[:intent_id])
    render partial: 'confirm_destroy', locals: { intent: @intent }
  end

  def destroy
    @agent = Agent.friendly.find(params[:agent_id])
    @intent = @agent.intents.friendly.find(params[:id])

    if @intent.destroy
      redirect_to user_agent_path(current_user, @agent), notice: t('views.intents.destroy.success_message', name: @intent.intentname)
    else
      redirect_to user_agent_path(current_user, @agent), alert: t(
          'views.intents.destroy.errors_message',
          errors: @intent.errors.full_messages.join(', ')
      )
    end
  end

  def confirm_destroy
    @agent = Agent.friendly.find(params[:agent_id])
    @intent = @agent.intents.friendly.find(params[:intent_id])
    render partial: 'confirm_destroy', locals: { intent: @intent }
  end

  def destroy
    @agent = Agent.friendly.find(params[:agent_id])
    @intent = @agent.intents.friendly.find(params[:id])

    if @intent.destroy
      redirect_to user_agent_path(current_user, @agent), notice: t('views.intents.destroy.success_message', name: @intent.intentname)
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
end
