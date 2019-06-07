class AgentsExportsController < ApplicationController
  include ActionController::Live

  before_action :set_owner_and_agent
  before_action :check_user_rights

  def full_export
    filename = "#{@agent.slug}.#{Time.current.strftime('%Y-%m-%d_%H-%M-%S')}.json"
    response.headers["Content-Disposition"] = "attachment; filename=\"#{filename}\""
    response.headers['Content-Type'] = 'application/json'
    begin
      Nlp::Package.new(@agent).full_json_export(response.stream)
    ensure
      response.stream.close
    end
  end


  private

    def check_user_rights
      case action_name
      when 'full_export'
        access_denied unless current_user.can? :show, @agent
      else
        access_denied
      end
    end

    def set_owner_and_agent
      begin
        @owner = User.friendly.find(params[:user_id])
        @agent = Agent.owned_by(@owner).friendly.find(params[:id])
      rescue ActiveRecord::RecordNotFound
        redirect_to '/404'
      end
    end
end
