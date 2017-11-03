class AgentMailer < ApplicationMailer

  def transfer_ownership(previous_owner, owner, agent)
    @agent = agent
    @previous_owner = previous_owner
    @owner = owner
    mail(
      to: @owner.email,
      subject: I18n.t('mailer.agent.transfer_ownership.subject')
    )
  end

end
