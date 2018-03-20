class FavoritesController < ApplicationController
  before_action :set_agent
  before_action :check_user_rights

  def create
    favorite = FavoriteAgent.new(user: current_user, agent: @agent)
    if favorite.save
      redirect_to user_agent_path(@agent.owner, @agent)
    else
      redirect_to user_agent_path(@agent.owner, @agent), alert: t('views.agents.favorite.add_errors_message',
                                        errors: favorite.errors.full_messages.join(', '))
    end
  end

  def destroy
    favorite = FavoriteAgent.find(params[:id])
    if favorite.destroy
      redirect_to user_agent_path(favorite.agent.owner, favorite.agent)
    else
      redirect_to user_agent_path(@agent.owner, @agent), alert: t('views.agents.favorite.remove_errors_message',
                                        errors: favorite.errors.full_messages.join(', '))
    end
  end

  private

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
