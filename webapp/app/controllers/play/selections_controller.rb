class Play::SelectionsController < ApplicationController

  def edit
    user_state = UserUiState.new(current_user)
    @selected_agent_ids = user_state.play_agents_selection
    @search = PlayAgentsSearch.new(current_user)
    @agents = Agent.search(@search.options).order(name: :asc)
    render partial: 'edit'
  end

  def search
    respond_to do |format|
      format.js {
        search = PlayAgentsSearch.new(current_user, search_params)
        agents = Agent.search(search.options).order(name: :asc)
        @list = render_to_string(partial: 'list', locals: {
          agents: agents,
          selected_agent_ids: search.selected_ids
        })
      }
    end
  end

  def update
    selection = selection_params['agent_ids'].reject { |id| id.blank? }[0,10]
    user_state = UserUiState.new(current_user)
    user_state.play_agents_selection = selection
    user_state.play_search = user_state.play_search.merge({ agent_ids: selection })
    user_state.save
    redirect_to "/play"
  end


  private

    def selection_params
      params.require(:selection).permit(agent_ids: [])
    end

    def search_params
      params.require(:search).permit(:query, :filter_owner, :selected, selected_ids: [])
    end

end
