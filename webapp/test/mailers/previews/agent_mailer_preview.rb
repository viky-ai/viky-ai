class AgentMailerPreview < ActionMailer::Preview

  def transfer_ownership
    AgentMailer.transfer_ownership(User.first, User.last, Agent.first)
  end

end
