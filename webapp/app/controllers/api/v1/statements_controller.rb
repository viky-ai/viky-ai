class Api::V1::StatementsController < Api::V1::ApplicationController
  skip_before_action :verify_authenticity_token

  def create
    chat_session = ChatSession.find(params[:id])
    if chat_session.expired?
      render json: { errors: t('statements.expired_session') }, status: :forbidden
    else
      statement = ChatStatement.new(statement_params)
      statement.speaker = :bot
      statement.chat_session = chat_session
      statement.save
      head :created
    end
  end

  private
    def statement_params
      params.require(:statement).permit(:content, :nature)
    end
end
