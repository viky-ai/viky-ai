class PlayController < ApplicationController
  def index
    user_state = UserUiState.new(current_user)
    available_agents = Agent.where(id: user_state.play_agents_selection).order(name: :asc)

    if params[:play_interpreter].nil?
      @interpreter = PlayInterpreter.new(user_state.play_search)
      @interpreter.available_agents = available_agents
      @interpreter.valid? ? @interpreter.proceed : @interpreter.errors.clear
    else
      @interpreter = PlayInterpreter.new(play_params)
      @interpreter.available_agents = available_agents
      save_state(@interpreter, user_state)

      respond_to do |format|
        format.js {
          if @interpreter.valid?
            @interpreter.proceed
            @form = render_to_string(partial: 'form', locals: { interpreter: @interpreter })
            @result = render_to_string(partial: "result", locals: { interpreter: @interpreter })
            render partial: 'interpret_succeed'
          else
            @form = render_to_string(partial: 'form', locals: { interpreter: @interpreter })
            render partial: 'interpret_failed'
          end
        }
      end
    end
  end

  def reset
    user_state = UserUiState.new(current_user)
    user_state.play_search = {
      agent_ids: user_state.play_search["agent_ids"],
      text: "",
      language: "*",
      spellchecking: "low"
    }
    user_state.save
    redirect_to action: :index
  end


  private

    def play_params
      params.require(:play_interpreter).permit(:text, :language, :spellchecking, agent_ids: [] )
    end

    def save_state(interpreter, user_state)
      user_state.play_search = {
        agent_ids: interpreter.agent_ids,
        text: interpreter.text,
        language: interpreter.language,
        spellchecking: interpreter.spellchecking
      }
      user_state.save
    end

end
