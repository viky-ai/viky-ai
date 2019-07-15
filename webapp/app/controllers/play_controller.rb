class PlayController < ApplicationController

  def index
    user_state = UserUiState.new(current_user)
    @selected_agent_ids = user_state.play_agents_selection
    @available_agents = Agent.where(id: @selected_agent_ids).order(name: :asc)
    @agent = current_agent
  end


  private

    def current_agent
      if !params[:ownername].nil? && !params[:agentname].nil?
        begin
          owner = User.friendly.find(params[:ownername])
          @agent = Agent.owned_by(owner).friendly.find(params[:agentname])
          return @agent
        rescue ActiveRecord::RecordNotFound
          redirect_to '/404'
        end
      else
        return @available_agents.first unless @available_agents.empty?
      end
    end

end
