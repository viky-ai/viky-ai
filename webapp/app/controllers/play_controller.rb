class PlayController < ApplicationController
  def index
    user_state = UserUiState.new(current_user)
    agents = Agent.where(id: user_state.play_agents_selection).order(name: :asc)

    if params[:play_interpreter].nil?
      @interpreter = PlayInterpreter.new
      @interpreter.agents = agents
      unless @interpreter.agents.empty?
        @interpreter.text = user_state.play_search.dig('text')
        @interpreter.language = user_state.play_search.dig('language')
        @interpreter.spellchecking = user_state.play_search.dig('spellchecking')
        @interpreter.valid? ? @interpreter.proceed : @interpreter.errors.clear
      end
    else
      @interpreter = PlayInterpreter.new(play_params)
      @interpreter.agents = agents

      user_state.play_search = {
        text: @interpreter.text,
        language: @interpreter.language,
        spellchecking: @interpreter.spellchecking
      }
      user_state.save

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
  end

  def reset
    user_state = UserUiState.new(current_user)
    user_state.play_search = {
      text: "",
      language: "*",
      spellchecking: "low"
    }
    user_state.save
    redirect_to action: :index
  end


  private

    def play_params
      params.require(:play_interpreter).permit(:agent_id, :text, :language, :spellchecking)
    end

end
