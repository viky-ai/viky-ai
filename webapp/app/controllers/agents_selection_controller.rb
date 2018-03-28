class AgentsSelectionController < ApplicationController
  before_action :set_owner
  before_action :set_agent
  before_action :check_user_rights

  def index
    @search = AgentSelectSearch.new(current_user, search_params)
    @available_destinations = @agent.available_destinations(@search.options).order(name: :asc)
    render partial: 'select_destination', locals: {
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
