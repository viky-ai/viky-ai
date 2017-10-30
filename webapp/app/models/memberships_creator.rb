class MembershipsCreator

  attr_reader :errors, :new_collaborators

  def initialize(agent, users_ids, rights)
    @agent = agent
    @users_ids = users_ids
    @rights = rights

    @errors = []
    @new_collaborators = []
  end

  def valid?
    @errors.empty?
  end

  def create
    validate_users_ids_required
    if valid?
      find_users
      if valid?
        new_memberships = @new_collaborators.collect do |user|
          Membership.new(user_id: user.id, agent_id: @agent.id, rights: @rights)
        end
        atomic_save(new_memberships)
      end
      send_emails if valid?
    end
    valid?
  end

  private

    def validate_users_ids_required
      @errors << I18n.t('views.memberships.new.empty_dest_message') if @users_ids.empty?
    end

    def find_users
      begin
        @new_collaborators = User.find(@users_ids)
      rescue
        @new_collaborators = User.where(id: @users_ids)
        @errors << I18n.t('views.memberships.new.unknown_user')
      end
    end

    def atomic_save(new_memberships)
      invalid_user = []
      ActiveRecord::Base.transaction do
        new_memberships.each do |membership|
          if !membership.save
            invalid_user << User.find(membership.user_id).username
          end
        end
        raise ActiveRecord::Rollback unless invalid_user.empty?
      end
      @errors << I18n.t('views.memberships.new.fail_message', users: invalid_user.join(', ')) if !invalid_user.empty?
    end

    def send_emails
      @new_collaborators.each do |collaborator|
        MembershipMailer.create_membership(@agent.owner, @agent, collaborator).deliver_later
      end
    end

end
