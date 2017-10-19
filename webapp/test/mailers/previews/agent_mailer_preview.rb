class AgentMailerPreview < ActionMailer::Preview

  def transfert_ownership
    AgentMailer.transfert_ownership(User.first, User.last, Agent.first)
  end

end
