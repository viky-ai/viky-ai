class AliasedInterpretationsController < ApplicationController
  before_action :set_owner
  before_action :set_agent
  before_action :check_user_rights

  def index
    referrer = Rails.application.routes.recognize_path(URI(request.referer).path)
    @user_comes_from_index = referrer[:action] == 'index'

    if referrer[:controller] == 'entities_lists'
      @entities_list = @agent.entities_lists.friendly.find(params[:entities_list_id])
      @interpretations = @entities_list.aliased_interpretations
    else
      @interpretation = @agent.interpretations.friendly.find(params[:interpretation_id])
      @interpretations = @interpretation.aliased_interpretations
    end

    render layout: false
  end


  private

    def set_owner
      @owner = User.friendly.find(params[:user_id])
    end

    def set_agent
      @agent = Agent.owned_by(@owner).friendly.find(params[:agent_id])
    end

    def check_user_rights
      access_denied unless current_user.can? :show, @agent
    end
end
