class MembershipMailer < ApplicationMailer

  def create_membership(owner, agent, new_collaborator)
    @agent = agent
    @owner = owner
    @new_collaborator = new_collaborator
    mail(
      to: @new_collaborator.email,
      subject: I18n.t('mailer.membership.create_membership.subject')
    )
  end

end
