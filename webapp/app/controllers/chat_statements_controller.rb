class ChatStatementsController < ApplicationController

  def create
    @bot = Bot.find(params[:chatbot_id])
    @chat_session = ChatSession.where(user: current_user, bot: @bot).last
    @content = statement_params[:content]

    @chat_statement = ChatStatement.new
    @chat_statement.chat_session = @chat_session
    @chat_statement.speaker = :user
    @chat_statement.nature = :text
    @chat_statement.content = @content

    respond_to do |format|
      format.js {
        unless @content.blank?
          @chat_statement.save
          @html = render_to_string(
            partial: @chat_statement.nature,
            locals: { statement: @chat_statement }
          )
          render partial: 'create_succeed'
        end
      }
    end
  end

  private

    def statement_params
      params.require(:statement).permit(:content)
    end

end
