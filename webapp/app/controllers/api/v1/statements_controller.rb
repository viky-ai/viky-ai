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

    def text_content_params
      [:text, speech: [:text, :locale]]
    end

    def image_content_params
      [:url, :title, :subtitle, speech: [:text, :locale]]
    end

    def video_content_params
      [:params, :title, :subtitle, speech: [:text, :locale]]
    end

    def button_content_params
      [:text, payload: {}, speech: [:text, :locale]]
    end

    def button_group_content_params
      [:disable_on_click, buttons: [ :text, payload: {} ], speech: [:text, :locale]]
    end

    def list_items_content_params
      (
        text_content_params +
        image_content_params +
        button_content_params +
        button_group_content_params
      ).uniq
    end

    def statement_params
      nature = params[:statement][:nature]
      case nature
      when 'text'
        params.require(:statement).permit(:nature, content: text_content_params)
      when 'image'
        params.require(:statement).permit(:nature, content: image_content_params)
      when 'video'
        params.require(:statement).permit(:nature, content: video_content_params)
      when 'button'
        params.require(:statement).permit(:nature, content: button_content_params)
      when 'button_group'
        params.require(:statement).permit(:nature, content: button_group_content_params)
      when 'list'
        params.require(:statement).permit(
          :nature,
          content: [
            :orientation,
            items: [:nature, content: list_items_content_params],
            speech: [:text, :locale]
          ]
        )
      when 'card'
        params.require(:statement).permit(
          :nature,
          content: [
            components: [
              :nature, content: list_items_content_params
            ]
          ]
        )
      end
    end
end
