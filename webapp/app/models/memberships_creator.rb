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
    find_users
    if valid?
      new_memberships = @users_ids.collect do |user_id|
        Membership.new(user_id: user_id, agent_id: @agent.id, rights: @rights)
      end
      atomic_save(new_memberships)
      send_emails if valid?
    end
    valid?
  end

  private

    def validate_users_ids_required
      @errors << I18n.t('views.memberships.new.empty_dest_message') if @users_ids.empty?
    end

    def find_users
      @new_collaborators = User.where(id: @users_ids)
    end

    def atomic_save(new_memberships)
      ActiveRecord::Base.transaction do
        new_memberships.each do |membership|
          if !membership.save
            if membership.errors[:user].any? || membership.errors[:agent].any? || membership.errors[:rights].any?
              @errors << "User #{membership.errors[:user].join(', ')}" if membership.errors[:user].any?
              @errors << "Agent #{membership.errors[:agent].join(', ')}" if membership.errors[:agent].any?
              @errors << "Rights #{membership.errors[:rights].join(', ')}" if membership.errors[:rights].any?
            else
              @errors << I18n.t('views.memberships.new.fail_message', user: membership.user.username)
            end
          end
        end
        raise ActiveRecord::Rollback unless @errors.empty?
      end
    end

    def send_emails
      @new_collaborators.each do |collaborator|
        MembershipMailer.create_membership(@agent.owner, @agent, collaborator).deliver_later
      end
    end

end
