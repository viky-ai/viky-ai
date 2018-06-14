class Api::V1::ChatStatementsController < Api::V1::ApplicationController
  skip_before_action :verify_authenticity_token

  def create
    chat_session = ChatSession.find(params[:chat_session_id])
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
        params.require(:statement).permit(:nature, content: text_content_params)
      when 'image'
        params.require(:statement).permit(:nature, content: image_content_params)
      when 'video'
        params.require(:statement).permit(:nature, content: video_content_params)
      when 'map'
        params.require(:statement).permit(:nature, content: map_content_params)
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
          content: card_content_params
        )
      end
    end

    def text_content_params(with_speech=true)
      speech = with_speech ? [speech: [:text, :locale]] : []
      [:text] + speech
    end

    def image_content_params(with_speech=true)
      speech = with_speech ? [speech: [:text, :locale]] : []
      [:url, :title, :description] + speech
    end

    def video_content_params(with_speech=true)
      speech = with_speech ? [speech: [:text, :locale]] : []
      [:params, :title, :description] + speech
    end

    def map_content_params(with_speech=true)
      speech = with_speech ? [speech: [:text, :locale]] : []
      [:title, :description, params: [
        :api_key, :endpoint, :query
      ]] + speech
    end

    def button_content_params(with_speech=true)
      speech = with_speech ? [speech: [:text, :locale]] : []
      [:text, :href, payload: {}, ] + speech
    end

    def button_group_content_params(with_speech=true)
      speech = with_speech ? [speech: [:text, :locale]] : []
      [:disable_on_click, buttons: [ :text, payload: {} ]] + speech
    end

    def card_content_params
      [
        components: [
          :nature,
          content: (
            text_content_params(false) +
            image_content_params(false) +
            video_content_params(false) +
            map_content_params(false) +
            button_content_params(false) +
            button_group_content_params(false)
          ).uniq
        ]
      ] + [speech: [:text, :locale]]
    end

    def list_items_content_params
      (
        text_content_params(false) +
        image_content_params(false) +
        video_content_params(false) +
        map_content_params(false) +
        button_content_params(false) +
        button_group_content_params(false) +
        card_content_params
      ).uniq
    end
end
