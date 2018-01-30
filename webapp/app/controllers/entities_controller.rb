class EntitiesController < ApplicationController
  before_action :set_agent
  before_action :check_user_rights

  def create
    entities_list = @agent.entities_lists.friendly.find(params[:entities_list_id])
    entity = Entity.new(entity_params)
    entity.entities_list = entities_list
    respond_to do |format|
      if entity.save
        format.js do
          @html_form = render_to_string(partial: 'form', locals: { agent: @agent, entities_list: entities_list, entity: Entity.new})
          render partial: 'create_succeed'
        end
      else
        format.js do
          @html_form = render_to_string(partial: 'form', locals: { agent: @agent, entities_list: entities_list, entity: entity})
          render partial: 'create_failed'
        end
      end
    end
  end

  private

    def entity_params
      params.require(:entity).permit(:auto_solution_enabled, :terms, :solution)
    end

    def set_agent
      @agent = Agent.friendly.find(params[:agent_id])
    end

    def check_user_rights
      case action_name
        when 'create'
          access_denied unless current_user.can? :edit, @agent
        else
          access_denied
      end
    end
end
