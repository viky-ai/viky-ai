class Api::V1::NlpController < Api::V1::ApplicationController
  def interpret
    @nlp = Nlp::Interpret.new(interpret_parameters)

    @nlp.agent_token = request.headers["Agent-Token"]     if @nlp.agent_token.blank?
    @nlp.language    = request.headers["Accept-Language"] if @nlp.language.blank?

    respond_to do |format|
      format.json {
        if @nlp.valid?
          @response = @nlp.proceed
          unless @response[:status] == '200'
            render json: @response[:body], status: @response[:status]
          end
        else
          render json: { errors: @nlp.errors.full_messages }, status: 422
        end
      }
    end
  end


  private

    def interpret_parameters
      params.permit(
        :ownername, :agentname, :format, :sentence, :language, :agent_token
      )
    end

end
