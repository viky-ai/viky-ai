class MembershipsCreator
  attr_reader :errors, :new_collaborators

  def initialize(agent, user_identities, rights)
    @agent = agent
    @user_identities = user_identities.split(/[;,\,]/).map(&:strip).reject(&:empty?)
    @rights = rights

    @errors = []
    @new_collaborators = []
  end

  def valid?
    @errors.empty?
  end

  def create
    validate_users_required
    find_users
    if valid?
      new_memberships = @new_collaborators.collect do |user|
        Membership.new(user_id: user.id, agent_id: @agent.id, rights: @rights)
      end
      atomic_save(new_memberships)
      send_emails if valid?
    end
    valid?
  end

  private

    def validate_users_required
      @errors << I18n.t('views.memberships.new.empty_dest_message') if @user_identities.empty?
    end

    def find_users
      @user_identities.each do |identity|
        user = User.find_by('email = ? OR username = ?', identity, identity)
        if user.nil?
          @errors << I18n.t('views.memberships.new.unknown_user', user: identity)
        else
          @new_collaborators << user
        end
      end
    end

    def atomic_save(new_memberships)
      ActiveRecord::Base.transaction do
        new_memberships.each do |membership|
          unless membership.save
            if membership.errors[:user].any? || membership.errors[:agent].any? || membership.errors[:rights].any?
              @errors << "User #{membership.errors[:user].join(', ')}" if membership.errors[:user].any?
              @errors << "Agent #{membership.errors[:agent].join(', ')}" if membership.errors[:agent].any?
              @errors << "Rights #{membership.errors[:rights].join(', ')}" if membership.errors[:rights].any?
            elsif membership.errors[:user_id].any?
              @errors << I18n.t('views.memberships.new.duplicate_user', user: selected_user_identity(membership.user))
            else
              @errors << I18n.t('views.memberships.new.fail_message', user: selected_user_identity(membership.user))
            end
            @new_collaborators.delete(membership.user)
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

    def selected_user_identity(user)
      [user.email, user.username].select{ |identity| @user_identities.include? identity }.first
    end

end
