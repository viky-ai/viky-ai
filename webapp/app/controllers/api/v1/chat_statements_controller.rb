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
        data = params.require(:statement).permit(:nature, content: map_content_params)
        transform_map_to_interactive_map(data)
      when 'button'
        params.require(:statement).permit(:nature, content: button_content_params)
      when 'button_group'
        params.require(:statement).permit(:nature, content: button_group_content_params)
      when 'list'
        data = params.require(:statement).permit(
          :nature,
          content: [
            :orientation,
            items: [:nature, content: list_items_content_params],
            speech: [:text, :locale]
          ]
        )
        transform_map_to_interactive_map(data)
      when 'card'
        data = params.require(:statement).permit(
          :nature,
          content: card_content_params
        )
        transform_map_to_interactive_map(data)
      when 'geolocation'
        params.require(:statement).permit(:nature, content: geolocation_content_params)
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
        :api_key, :endpoint, :query, payload: {}
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

    def geolocation_content_params(with_speech=true)
      speech = with_speech ? [speech: [:text, :locale]] : []
      [:text] + speech
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
            button_group_content_params(false) +
            geolocation_content_params(false)
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
        card_content_params +
        geolocation_content_params(false)
      ).uniq
    end

    # enum value "map" is not permitted due to conflict with ActiveRecord map method
    # so we use interactive_map internally.
    def transform_map_to_interactive_map(data)
      # Map
      data[:nature] = "interactive_map" if data[:nature] == "map"

      # Card with map
      data = transform_map_to_interactive_map_in_card(data)

      # List with map
      if data.dig(:content, :items).respond_to? "each_with_index"
        data[:content][:items].each_with_index do |item, i|
          if item[:nature] == "map"
            data[:content][:items][i][:nature] = "interactive_map"
          end
          if item[:nature] == "card"
            data[:content][:items][i] = transform_map_to_interactive_map_in_card(item)
          end
        end
      end

      data
    end

    def transform_map_to_interactive_map_in_card(data)
      if data.dig(:content, :components).respond_to? "each_with_index"
        data[:content][:components].each_with_index do |component, i|
          if component[:nature] == "map"
            data[:content][:components][i][:nature] = "interactive_map"
          end
        end
      end
      data
    end

end
