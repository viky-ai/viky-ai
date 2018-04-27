class ChatbotsController < ApplicationController
  before_action :set_bots
  before_action :set_available_recognition_locale
  before_action :set_current_recognition_locale

  def index
  end

  def show
    @bot = Bot.find(params[:id])
    if ChatSession.where(user: current_user, bot: @bot).exists?
      @chat_session = ChatSession.where(user: current_user, bot: @bot).last
    else
      @chat_session = ChatSession.new(user: current_user, bot: @bot)
      @chat_session.save
    end
  end

  def reset
    @bot = Bot.find(params[:id])

    previous_chat_session = ChatSession.where(user: current_user, bot: @bot).last
    chat_session = ChatSession.new(user: current_user, bot: @bot)
    chat_session.save

    ActionCable.server.broadcast "chat_session_channel_#{current_user.id}", {
      session_id: previous_chat_session.id,
      action: "reset",
      path: chatbot_path(@bot, recognition_locale: @current_recognition_locale)
    }

    redirect_to chatbot_path(@bot, recognition_locale: @current_recognition_locale)
  end


  private

    def set_bots
      @bots = Bot.accessible_bots(current_user)
    end


    def set_current_recognition_locale
      @current_recognition_locale = "en-US"
      unless params[:recognition_locale].nil?
        if @available_recognition_locale.keys.include? params[:recognition_locale].to_sym
          @current_recognition_locale = params[:recognition_locale]
        end
      end
    end

    def set_available_recognition_locale
      @available_recognition_locale = ChatSession.locales
    end
end
