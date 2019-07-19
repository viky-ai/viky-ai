class PlayController < ApplicationController
  before_action :define_interpreter_and_check_rights, only: [:interpret]

  def index
    @interpreter = PlayInterpreter.new
    @interpreter.agents = interpreter_agents
    @interpreter.agent  = @interpreter.agents.first unless @interpreter.agents.empty?
  end

  def interpret
    respond_to do |format|
      format.js {
        if @interpreter.valid?
          @interpreter.proceed
          @aside = render_to_string(partial: 'aside', locals: { interpreter: @interpreter })
          @form = render_to_string(partial: 'form', locals: { interpreter: @interpreter })
          @result = render_to_string(partial: "result", locals: { interpreter: @interpreter })
          render partial: 'interpret_succeed'
        else
          @aside = render_to_string(partial: 'aside', locals: { interpreter: @interpreter })
          @form = render_to_string(partial: 'form', locals: { interpreter: @interpreter })
          render partial: 'interpret_failed'
        end
      }
    end
  end


  private

    def play_params
      params.require(:play_interpreter).permit(:text, :agentname, :ownername)
    end

    def interpreter_agents
      user_state = UserUiState.new(current_user)
      Agent.where(id: user_state.play_agents_selection).order(name: :asc)
    end

    def define_interpreter_and_check_rights
      @interpreter = PlayInterpreter.new(play_params)
      @interpreter.agents = interpreter_agents
      begin
        @interpreter.agent = Agent.owned_by(User.friendly.find(@interpreter.ownername)).friendly.find(@interpreter.agentname)
        access_denied and return unless current_user.can? :show, @interpreter.agent
      rescue ActiveRecord::RecordNotFound
        redirect_to '/404' and return
      end
    end

end
