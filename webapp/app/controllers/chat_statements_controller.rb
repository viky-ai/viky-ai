class ChatStatementsController < ApplicationController

  def create
    @bot = Bot.find(params[:chatbot_id])
    @chat_session = ChatSession.where(user: current_user, bot: @bot).last
    @content = statement_params[:content]

    @chat_statement = ChatStatement.new
    @chat_statement.chat_session = @chat_session
    @chat_statement.speaker = :user
    @chat_statement.nature = :text
    @chat_statement.content = {
      text: @content
    }

    respond_to do |format|
      format.js {
        if @chat_statement.save
          render partial: 'create_succeed'
        end
      }
    end
  end

  def user_action
    @bot = Bot.find(params[:chatbot_id])
    @chat_session = ChatSession.where(user: current_user, bot: @bot).last
    BotSendUserStatementJob.perform_later(@chat_session.id, 'button', params[:payload])
    respond_to do |format|
      format.js {
        render partial: 'user_action'
      }
    end
  end

  private

    def statement_params
      params.require(:statement).permit(:content)
    end

end
