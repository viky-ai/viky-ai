class MembershipsController < ApplicationController
  before_action :set_agent
  before_action :check_user_rights

  def index
    render partial: 'index', locals: { agent: @agent }
  end

  def new
    render partial: 'new', locals: { errors: [], users: "" }
  end

  def create
    memberships_creator = MembershipsCreator.new(@agent, memberships_params[:users], memberships_params[:rights])
    respond_to do |format|
      if memberships_creator.create
        users = memberships_creator.new_collaborators.collect { |user| user.username }.join(', ')
        format.js {
          @modal_content = render_to_string(
            partial: 'index',
            locals: { agent: @agent }
          )
          @agent_html = build_agent_content(@agent)
          @message = t('views.memberships.new.success_message', agent: @agent.agentname, users: users)
          render partial: 'create_succeed'
        }
      else
        format.js {
          @html = render_to_string(
            partial: 'new', locals: {
              users: memberships_params[:users],
              errors: memberships_creator.errors
            }
          )
          render partial: 'create_failed'
        }
      end
    end
  end

  def update
    @membership = Membership.find(params[:id])
    respond_to do |format|
      if @membership.update(membership_params)
        format.js{
          @modal_content = render_to_string(partial: 'edit', locals: { membership: @membership })
          @agent_html = build_agent_content(@agent)
          render partial: 'update_succeed'
        }
      else
        format.js{
          render partial: 'update_failed'
        }
      end
    end
  end

  def confirm_destroy
    @membership = Membership.find(params[:membership_id])
    render partial: 'confirm_destroy', locals: { membership: @membership }
  end

  def destroy
    @membership = Membership.find(params[:id])
    if @membership.destroy
      MembershipMailer.destroy_membership(@agent.owner, @agent, @membership.user).deliver_later if Feature.email_configured?
    end
    respond_to do |format|
      format.js{
        @modal_content = render_to_string(partial: 'index', formats: :html, locals: { agent: @agent })
        @agent_html = build_agent_content(@agent)
      }
    end
  end


  private

    def check_user_rights
      access_denied unless current_user.owner?(@agent)
    end

    def membership_params
      params.require(:membership).permit(:username, :rights)
    end

    def memberships_params
      params.require(:memberships).permit(:users, :rights)
    end

    def set_agent
      owner = User.friendly.find(params[:user_id])
      @agent = Agent.owned_by(owner).friendly.find(params[:agent_id])
    end

    def build_agent_content(agent)
      if params[:origin] == 'show'
        render_to_string(
          partial: '/agents/sharing',
          locals: { agent: agent }
        )
      else
        render_to_string(
          partial: '/agents/agent_box',
          locals: { agent: agent, editable: true }
        )
      end
    end

end
