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
      if statement.save
        head :created
      else
        render json: { errors: statement.errors.full_messages }, status: 422
      end
    end
  end


  private

    def statement_params
      nature = params[:statement][:nature]
      case nature
      when 'text'
        params.require(:statement).permit(
          :nature,
          content: [:text, speech: [:text, :locale]]
        )
      when 'image'
        params.require(:statement).permit(
          :nature,
          content: [:url, :title, :subtitle, speech: [:text, :locale]]
        )
      when 'button'
        params.require(:statement).permit(
          :nature,
          content: [:text, payload: {}, speech: [:text, :locale]]
        )
      when 'button_group'
        params.require(:statement).permit(
          :nature,
          content: [
            :disable_on_click,
            buttons: [ :text, payload: {} ],
            speech: [:text, :locale]
          ]
        )
      end
    end
end
