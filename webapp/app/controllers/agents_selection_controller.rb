class AgentsSelectionController < ApplicationController
  before_action :set_owner
  before_action :set_agent
  before_action :check_user_rights

  def index
    name = ''
    @search = AgentSelectSearch.new(current_user, search_params)
    if params[:from] == 'agents'
      @available_agents = @agent.available_successors(@search.options).order(name: :asc)
    else
      @available_agents = @agent.available_destinations(@search.options).order(name: :asc)
      name = params[:from] == 'intents' ? Intent.find(params[:current_id]).intentname : EntitiesList.find(params[:current_id]).listname
    end
    render partial: 'select_available_agent', locals: {
      name: name,
      from: params[:from],
      current_id: params[:current_id]
    }
  end


  private

    def set_owner
      @owner = User.friendly.find(params[:user_id])
    end

    def set_agent
      @agent = @owner.agents.friendly.find(params[:id])
    end

    def search_params
      params.permit(search: [:query, :filter_owner])[:search]
    end

    def check_user_rights
      case action_name
        when 'index'
          access_denied unless current_user.can? :show, @agent
        else
          access_denied
      end
    end
end
