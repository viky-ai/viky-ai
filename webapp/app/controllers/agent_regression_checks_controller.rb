class AgentRegressionChecksController < ApplicationController
  before_action :set_owner
  before_action :set_agent
  before_action :check_user_rights
  before_action :set_regression_check, only: [:update, :destroy]

  rescue_from ActiveRecord::RecordNotFound, with: :record_not_found

  def create
    @regression_check = AgentRegressionCheck.new(regression_check_params)
    @regression_check.agent = @agent

    if @regression_check.save
      @agent.run_regression_checks
      render json: {
        test: JSON.parse(@regression_check.to_json),
        tests_suite: JSON.parse(@agent.regression_checks_to_json)
      }, status: :created
    else
      render json: t('views.agent_regression_checks.new.failed_message'), status: :unprocessable_entity
    end
  end

  def run
    @agent.run_regression_checks
  end

  def update
    if @regression_check.update(regression_check_params)
      @regression_check.state = 'unknown'
      @regression_check.save
      @agent.run_regression_checks
      render json: {
        test: JSON.parse(@regression_check.to_json),
        tests_suite: JSON.parse(@agent.regression_checks_to_json)
      }, status: :ok
    else
      render json: t('views.agent_regression_checks.update.fail'), status: :unprocessable_entity
    end
  end

  def update_positions
    AgentRegressionCheck.update_positions(@agent, params[:ids])
    notify_ui
  end

  def destroy
    if @regression_check.destroy
      notify_ui
      render json: {
        test: JSON.parse(@regression_check.to_json),
        tests_suite: JSON.parse(@agent.regression_checks_to_json)
      }
    else
      render json: @regression_check.errors.messages[:base], status: :unprocessable_entity
    end
  end

  private

    def regression_check_params
      params.require(:regression_check)
            .permit(:sentence, :language, :spellchecking, :now, expected: [
              :package,
              :id,
              :solution,
              :root_type
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
        when 'create', 'destroy', 'update', 'update_positions'
          access_denied unless current_user.can? :edit, @agent
        else
          access_denied
      end
    end

    def notify_ui
      ActionCable.server.broadcast(
        "agent_regression_checks_channel_#{@agent.id}",
        agent_id: @agent.id,
        timestamp: Time.now.to_f * 1000,
        payload: JSON.parse(@agent.regression_checks_to_json)
      )
    end

    def record_not_found
      render json: t('views.agent_regression_checks.not_found'), status: :not_found
    end

end
