class Devise::MailerPreview < ActionMailer::Preview


  def confirmation_instructions
    CustomDeviseMailer.confirmation_instructions(User.first, "fake_token")
  end

  def reset_password_instructions
    CustomDeviseMailer.reset_password_instructions(User.first, "fake_token")
  end

  def unlock_instructions
    CustomDeviseMailer.unlock_instructions(User.first, "fake_token")
  end

  # def email_changed
  #   CustomDeviseMailer.email_changed(User.first)
  # end

  # def password_change
  #   CustomDeviseMailer.password_change(User.first)
  # end

  def invitation_instructions
    CustomDeviseMailer.invitation_instructions(User.first, "fake_token")
  end

end
