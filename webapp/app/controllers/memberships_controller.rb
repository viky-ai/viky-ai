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
    errors = []

    user_ids = memberships_params[:user_ids].split(';')
    dest_users = User.where(id: user_ids)

    search_initial_values = dest_users.map do |user|
      {
        user_id: user.id,
        name: user.name,
        username: user.username,
        email: user.email,
        images: user.image
      }
    end

    if dest_users.empty?
      errors << t('views.memberships.new.empty_dest_message')
    else
      memberships = dest_users.collect do |user|
        Membership.new(
          user_id: user.id,
          agent_id: @agent.id,
          rights: memberships_params[:rights]
        )
      end
      memberships_in_errors = []
      ActiveRecord::Base.transaction do
        memberships.each do |membership|
          if !membership.save
            memberships_in_errors << User.find(membership.user_id).username
          end
        end
        raise ActiveRecord::Rollback unless memberships_in_errors.empty?
      end
      if !memberships_in_errors.empty?
        errors << "Impossible to create share for : #{memberships_in_errors.join(', ')}"
      end
    end


    respond_to do |format|
      if errors.empty?
        users = dest_users.collect { |user| user.username }.join(', ')
        format.json{
          redirect_to agents_path, notice: t('views.memberships.new.success_message', agent: @agent.agentname, users: users)
        }
      else
        format.json{
          render json: {
            html: render_to_string(
              partial: 'new',
              formats: :html,
              locals: { search_initial_values: search_initial_values, errors: errors }),
            status: 422,
          }
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
            @users = User.confirmed
              .where.not(id: @agent.owner_id)
              .where("email LIKE ? OR username LIKE ?", "%#{query}%", "%#{query}%")
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
          @html = render_to_string(partial: 'edit', formats: :html, locals: { membership: @membership })
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
    @membership.destroy
    respond_to do |format|
      format.js{
        @html = render_to_string(partial: 'index', formats: :html, locals: { agent: @agent })
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
      @agent = owner.agents.friendly.find(params[:agent_id])
    end

end
