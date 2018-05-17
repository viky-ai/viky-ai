class Api::V1::ChatSessionsController < Api::V1::ApplicationController
  skip_before_action :verify_authenticity_token

  def update
    chat_session = ChatSession.find(params[:id])
    if chat_session.update(chat_session_params)
      head :ok
    else
      render json: { errors: chat_session.errors.full_messages }, status: 422
    end
  end

  private

    def chat_session_params
      params.require(:chat_session).permit(:locale)
    end
end
