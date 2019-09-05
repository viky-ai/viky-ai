module Feature

  def self.user_registration_disabled?
    ENV['VIKYAPP_USER_REGISTRATION'] == 'false'
  end

  def self.enable_user_registration
    ENV['VIKYAPP_USER_REGISTRATION'] = 'true'
  end

  def self.disable_user_registration
    ENV['VIKYAPP_USER_REGISTRATION'] = 'false'
  end

  def self.email_configured?
    smtp_enabled = ENV.fetch("SMTP_ENABLED") { false }
    postmark_enabled = ENV.fetch("POSTMARK_TOKEN") { false }
    postmark_enabled || smtp_enabled == 'true'
  end
end
