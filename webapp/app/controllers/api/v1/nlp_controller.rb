class Api::V1::NlpController < Api::V1::ApplicationController
  before_action :validate_owner_and_agent

  def interpret
    nlp = Nlp::Interpret.new(interpret_parameters)
    nlp.agent_token = request.headers["Agent-Token"]     if nlp.agent_token.blank?
    nlp.language    = request.headers["Accept-Language"] if nlp.language.blank?

    respond_to do |format|
      format.json {
        @body, @status = nlp.proceed
        render json: @body, status: @status unless @status == 200
      }
    end
  end


  private

    # Auto render 404.json on ActiveRecord::RecordNotFound exception
    def validate_owner_and_agent
      @owner = User.friendly.find(params[:ownername])
      @agent = Agent.owned_by(@owner).friendly.find(params[:agentname])
    end

    def interpret_parameters
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
