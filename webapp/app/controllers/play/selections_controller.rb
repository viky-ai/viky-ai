class Play::SelectionsController < ApplicationController

  def edit
    user_state = UserUiState.new(current_user)
    @selected_agent_ids = user_state.play_agents_selection
    @agents = Agent.search(user_id: current_user.id).order(name: :asc) #.page(params[:page]) #.per(10)
    render partial: 'edit'
  end

  def update
    selection = selection_params['agent_ids'].reject { |id| id.blank? }[0,10]
    user_state = UserUiState.new(current_user)
    user_state.play_agents_selection = selection
    user_state.save
    if selection.empty?
      redirect_to "/play"
    else
      agent = Agent.find(selection.first)
      redirect_to "/play/#{agent.slug}"
    end
  end

  private

  def selection_params
    params.require(:selection).permit(agent_ids: [])
  end

end
