class AgentMailer < ApplicationMailer

  def transfert_ownership(previous_owner, owner, agent)
    @agent = agent
    @previous_owner = previous_owner
    @owner = owner
    mail(
      to: @owner.email,
      subject: "test transfert ownership"
    )
  end

end
