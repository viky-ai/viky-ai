class Api::V1::StatementsController < Api::V1::ApplicationController
  skip_before_action :verify_authenticity_token

  def create
    statement = ChatStatement.new(statement_params)
    statement.speaker = :bot
    statement.chat_session = ChatSession.find(params[:id])
    statement.save
    head :created
  end

  private
    def statement_params
      params.require(:statement).permit(:content, :nature)
    end
end
