class EntitiesListsController < ApplicationController
  before_action :set_agent
  before_action :check_user_rights

  def new
    @entities_list = EntitiesList.new
    render partial: 'new'
  end

  def create
    @entities_list = EntitiesList.new(entities_list_params)
    @entities_list.agent = @agent
    respond_to do |format|
      if @entities_list.save
        format.json do
          redirect_to user_agent_path(current_user, @agent), notice: t('views.entities_lists.new.success_message')
        end
      else
        format.json do
          render json: {
            replace_modal_content_with: render_to_string(partial: 'new', formats: :html)
          }, status: 422
        end
      end
    end
  end

  private

    def entities_list_params
      params.require(:entities_list).permit(:listname, :description, :visibility)
    end

    def set_agent
      @agent = Agent.friendly.find(params[:agent_id])
    end

    def check_user_rights
      case action_name
        when 'new', 'create'
          access_denied unless current_user.can? :edit, @agent
        else
          access_denied
      end
    end
end
