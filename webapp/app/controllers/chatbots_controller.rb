class ChatbotsController < ApplicationController
  before_action :set_bot, except: [:index]
  before_action :set_available_recognition_locale
  before_action :check_user_rights, except: [:index]

  def index
    @search = ChatbotSearch.new(current_user, search_params)
    @bots_accessible = Bot.accessible_bots(current_user)
    @bots = Bot.search(@search.options).page(params[:page]).per(8)
    @search.save
  end

  def show
    @search = ChatbotSearch.new(current_user, search_params)
    @bots = Bot.search(@search.options).page(params[:page]).per(8)

    if ChatSession.where(user: current_user, bot: @bot).exists?
      @chat_session = ChatSession.where(user: current_user, bot: @bot).last
    else
      @chat_session = ChatSession.new(user: current_user, bot: @bot)
      @chat_session.save
    end

    # Update ChatSession locale if requested
    unless params[:recognition_locale].nil?
      if @available_recognition_locale.keys.include? params[:recognition_locale].to_sym
        @chat_session.locale = params[:recognition_locale]
        @chat_session.save
      end
    end
    @search.save
  end

  def reset
    previous_chat_session = ChatSession.where(user: current_user, bot: @bot).last
    chat_session = ChatSession.new(user: current_user, bot: @bot)
    chat_session.save

    ActionCable.server.broadcast "chat_session_channel_#{current_user.id}", {
      session_id: previous_chat_session.id,
      action: "reset",
      path: chatbot_path(@bot, recognition_locale: previous_chat_session.locale)
    }

    redirect_to chatbot_path(@bot, recognition_locale: previous_chat_session.locale)
  end


  private

    def check_user_rights
      case action_name
        when "show"
          access_denied unless current_user.can? :show, @bot.agent
        when "reset"
          access_denied unless current_user.can? :edit, @bot.agent
        else
          access_denied
      end
    end

    def set_bot
      @bot = Bot.find(params[:id])
    end

    def set_available_recognition_locale
      @available_recognition_locale = ChatSession.locales
    end

    def search_params
      params.permit(:id, search: [:query, :filter_wip])[:search]
    end
end
