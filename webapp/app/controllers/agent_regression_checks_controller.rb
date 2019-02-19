class AgentRegressionChecksController < ApplicationController
  before_action :set_owner
  before_action :set_agent
  before_action :set_regression_check, only: [:update, :destroy]
  before_action :check_user_rights

  rescue_from ActiveRecord::RecordNotFound, with: :record_not_found

  def create
    @regression_check = AgentRegressionCheck.new(regression_check_params)
    @regression_check.agent = @agent
    unless @regression_check.save
      redirect_to user_agent_path(@agent.owner, @agent),
        alert: t('views.agent_regression_checks.new.failed_message')
    end
    notify_ui
    render json: @regression_check.to_json, status: :created
  end

  def run
    @agent.run_regression_checks
  end

  def update
    if @regression_check.update(regression_check_params)
      @agent.run_regression_checks
      head :ok
    else
      render json: t('views.agent_regression_checks.update.fail'), status: :unprocessable_entity
    end
  end

  def destroy
    unless @regression_check.destroy
      render json: @regression_check.errors.messages[:base], status: :unprocessable_entity
      return
    end
    notify_ui
    head :no_content
  end

  private

    def regression_check_params
      params.require(:regression_check)
            .permit(:sentence, :language, :now, expected: [
              :package,
              :id,
              :solution
            ])
    end

    def set_owner
      @owner = User.friendly.find(params[:user_id])
    end

    def set_agent
      @agent = @owner.agents.friendly.find(params[:agent_id])
    end

    def set_regression_check
      @regression_check = AgentRegressionCheck.find(params[:id])
    end

    def check_user_rights
      case action_name
        when 'run'
          access_denied unless current_user.can? :show, @agent
        when 'create', 'destroy', 'update'
          access_denied unless current_user.can? :edit, @agent
        else
          access_denied
      end
    end

    def notify_ui
      ActionCable.server.broadcast(
        'agent_regression_checks_channel',
        agent_id: @agent.id,
        timestamp: Time.now.to_f * 1000,
        payload: JSON.parse(@agent.regression_checks_to_json)
      )
    end

    def record_not_found
      render json: t('views.agent_regression_checks.not_found'), status: :not_found
    end

end
