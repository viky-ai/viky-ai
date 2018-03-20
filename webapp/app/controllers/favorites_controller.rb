class FavoritesController < ApplicationController
  before_action :set_agent
  before_action :check_user_rights

  def create
    favorite = FavoriteAgent.new(user: current_user, agent: @agent)
    respond_to do |format|
      format.js {
        if favorite.save
          @html = generate_agents_header_content
          render partial: 'create_succeed'
        else
          @message = t(
            'views.agents.favorite.add_errors_message',
            errors: favorite.errors.full_messages.join(', ')
          )
          render partial: 'create_failed'
        end
      }
    end
  end

  def destroy
    favorite = FavoriteAgent.find(params[:id])
    respond_to do |format|
      format.js {
        if favorite.destroy
          @html = generate_agents_header_content
          render partial: 'destroy_succeed'
        else
          @message = t(
            'views.agents.favorite.remove_errors_message',
            errors: favorite.errors.full_messages.join(', ')
          )
          render partial: 'destroy_failed'
        end
      }
    end
  end


  private

    def generate_agents_header_content
      render_to_string(
        partial: 'agents/header_content',
        locals: { agent: @agent }
      )
    end

    def set_agent
      begin
        if params[:agent_id].present?
          @agent = Agent.find(params[:agent_id])
        else
          @agent = FavoriteAgent.find(params[:id]).agent
        end
      rescue ActiveRecord::RecordNotFound
        redirect_to '/404'
      end
    end

    def check_user_rights
      case action_name
      when 'create', 'destroy'
        access_denied unless current_user.can? :show, @agent
      else
        access_denied
      end
    end
end
