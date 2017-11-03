class MembershipMailerPreview < ActionMailer::Preview

  def create_membership
    agent = Agent.first
    collaborator = Membership.where.not(agent_id: agent.id).first.user
    MembershipMailer.create_membership(agent.owner, agent, collaborator)
  end

  def destroy_membership
    agent = Agent.first
    collaborator = Membership.where.not(agent_id: agent.id).first.user
    MembershipMailer.destroy_membership(agent.owner, agent, collaborator)
  end

end
