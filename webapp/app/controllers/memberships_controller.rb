class MembershipsController < ApplicationController
  before_action :set_agent
  before_action :check_user_rights

  def index
    render partial: 'index'
  end

  def new
    @membership = Membership.new()
    render partial: 'new'
  end

  def create
    @membership = Membership.new(membership_params)
    @membership.agent_id = @agent.id

    respond_to do |format|
      if @membership.save
        format.json{
          redirect_to agents_path, notice: "done"
        }
      else
        format.json{
          render json: {
            html: render_to_string(partial: 'new', formats: :html),
            status: 422
          }
        }
      end
    end
  end

  def update
    membership = Membership.find(params[:id])
    respond_to do |format|
      if membership.update(membership_params)
        format.js{
          render partial: 'update_succeed'
        }
      else
        format.js{
          render partial: 'update_failed'
        }
      end
    end
  end


  private

    def check_user_rights
      access_denied unless current_user.owner?(@agent)
    end

    def membership_params
      params.require(:membership).permit(:user_id, :rights)
    end

    def set_agent
      user = User.friendly.find(params[:user_id])
      @agent = user.agents.friendly.find(params[:agent_id])
    end
end
