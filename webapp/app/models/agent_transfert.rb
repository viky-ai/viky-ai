class AgentTransfert

  attr_accessor :errors, :agent, :previous_owner, :new_owner

  def initialize(agent, new_owner_username)
    @errors = []
    @agent = agent
    @new_owner_username = new_owner_username

    @previous_owner = User.find(@agent.owner_id)
    @new_owner = User.find_by_username(@new_owner_username)
  end

  def valid?
    @errors.empty?
  end

  def proceed
    validate
    if valid?
      ActiveRecord::Base.transaction do
        Membership.where(user_id: @agent.owner_id).where(agent_id: @agent.id).first.destroy
        new_membership = Membership.new(agent_id: @agent.id, user_id: @new_owner.id)
        if new_membership.save
          @agent.owner_id = @new_owner.id
          unless @agent.save
            @errors << @agent.errors.full_messages
            raise ActiveRecord::Rollback
          end
        else
          @errors << new_membership.errors.full_messages
          raise ActiveRecord::Rollback
        end
      end
      AgentMailer.transfert_ownership(@previous_owner, @new_owner, @agent).deliver_later
    end
  end

  private

    def validate
      @errors = []
      validate_new_owner_username
      validate_agent_slug_availability if valid?
    end

    def validate_new_owner_username
      if @new_owner.nil?
        @errors << I18n.t('errors.agent.transfer_ownership.invalid_username')
      end
    end

    def validate_agent_slug_availability
      if Agent.where(owner_id: @new_owner.id, agentname: @agent.agentname).count != 0
        @errors << I18n.t('errors.agent.transfer_ownership.duplicate_agentname')
      end
    end

end
