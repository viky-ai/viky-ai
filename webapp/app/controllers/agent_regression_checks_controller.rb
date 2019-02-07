class AgentRegressionChecksController < ApplicationController
  before_action :set_owner
  before_action :set_agent
  before_action :check_user_rights

  def create
    @regression_check = AgentRegressionCheck.new(regression_check_params)
    @regression_check.agent = @agent
    unless @regression_check.save
      redirect_to user_agent_path(@agent.owner, @agent),
        alert: t('views.agent_regression_checks.new.failed_message')
    end
  end

  def run
    @agent.run_tests
  end

  def destroy
    @regression_check  = AgentRegressionCheck.find_by_id(params[:id])
    if @regression_check.nil?
      render json: t('views.agent_regression_checks.not_found'), status: :not_found
    else
      unless @regression_check.destroy
        render json: @regression_check.errors.messages[:base], status: :unprocessable_entity
        return
      end
      render 'create'
    end
  end

  private

    def regression_check_params
      params.require(:regression_check)
            .permit(:sentence, :language, :now, expected: [
              :package,
              :id,
              :score,
              :solution
            ])
    end

    def set_owner
      @owner = User.friendly.find(params[:user_id])
    end

    def set_agent
      @agent = @owner.agents.friendly.find(params[:agent_id])
    end

    def check_user_rights
      case action_name
        when 'run'
          access_denied unless current_user.can? :show, @agent
        when 'create', 'destroy'
          access_denied unless current_user.can? :edit, @agent
        else
          access_denied
      end
    end
end
