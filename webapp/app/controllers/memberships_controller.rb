class MembershipsController < ApplicationController
  before_action :set_agent
  before_action :check_user_rights

  def index
    render partial: 'index', locals: { agent: @agent }
  end

  def new
    @membership = Membership.new
    render partial: 'new', locals: { search_initial_values: [], errors: [] }
  end

  def create
    memberships_creator = MembershipsCreator.new(@agent, memberships_params[:user_ids].split(';'), memberships_params[:rights])
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
        search_initial_values = memberships_creator.new_collaborators.collect do |user|
          {
            user_id: user.id,
            name: user.name,
            username: user.username,
            email: user.email,
            images: user.image
          }
        end
        format.js {
          @html = render_to_string(
            partial: 'new',
            locals: { search_initial_values: search_initial_values, errors: memberships_creator.errors }
          )
          render partial: 'create_failed'
        }
      end
    end
  end

  def search_users_to_share_agent
    respond_to do |format|
      format.json {
        query = params[:q].strip
        @users = []
        unless query.nil?
          if query.size > 2
            collaborators_ids = @agent.collaborators.collect { |c| c.id }
            @users = User.confirmed
              .where.not(id: @agent.owner_id)
              .where("email LIKE ? OR username LIKE ?", "%#{query}%", "%#{query}%")
              .where.not(id: collaborators_ids)
              .limit(10)
          end
        end
      }
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
      MembershipMailer.destroy_membership(@agent.owner, @agent, @membership.user).deliver_later
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
      params.require(:memberships).permit(:user_ids, :rights)
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
