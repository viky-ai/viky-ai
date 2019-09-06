class Api::V1::NlpController < Api::V1::ApplicationController
  before_action :validate_owner_and_agent

  def interpret
    nlp = Nlp::Interpret.new(interpret_parameters)
    nlp.language = request.headers["Accept-Language"] if nlp.language.blank?

    respond_to do |format|
      format.json {
        if !validate_token
          render json: { errors: [I18n.t('controllers.api.access_denied')] }, status: 401
        else
          @body, @status = nlp.proceed
          render json: @body, status: @status unless @status == 200
        end
      }
    end
  end


  private

    # Auto render 404.json on ActiveRecord::RecordNotFound exception
    def validate_owner_and_agent
      @owner = User.friendly.find(request_parameters[:ownername])
      @agent = Agent.owned_by(@owner).friendly.find(request_parameters[:agentname])
    end

    def validate_token
      agent_token = request_parameters[:agent_token].present? ? request_parameters[:agent_token] : request.headers["Agent-Token"]
      @agent.api_token == agent_token
    end

    def interpret_parameters
      p = request_parameters
      p.delete(:ownername)
      p.delete(:agentname)
      p.delete(:agent_token)
      p.merge(agents: [@agent])
    end

    def request_parameters
      params.permit(
        :ownername,
        :agentname,
        :format,
        :sentence,
        :language,
        :agent_token,
        :verbose,
        :now,
        :spellchecking,
        context: {}
      )
    end
end
