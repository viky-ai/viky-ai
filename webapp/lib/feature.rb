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
    return true if Rails.env.test?

    smtp_enabled = ENV.fetch("SMTP_ENABLED") { false }
    postmark_enabled = ENV.fetch("POSTMARK_TOKEN") { false }
    postmark_enabled || smtp_enabled == 'true'
  end

  def self.quota_enabled?
    ENV['VIKYAPP_QUOTA_ENABLED'] == 'true'
  end

  def self.enable_quota
    ENV['VIKYAPP_QUOTA_ENABLED'] = 'true'
  end

  def self.disable_quota
    ENV['VIKYAPP_QUOTA_ENABLED'] = 'false'
  end

  def self.with_quota_enabled
    enable_quota
    yield
    disable_quota
  end

  def self.chatbot_enabled?
    ENV['VIKYAPP_CHATBOT_ENABLED'] == 'true'
  end

  def self.enable_chatbot
    ENV['VIKYAPP_CHATBOT_ENABLED'] = 'true'
  end

  def self.disable_chatbot
    ENV['VIKYAPP_CHATBOT_ENABLED'] = 'false'
  end

  def self.with_chatbot_enabled
    enable_chatbot
    yield
    disable_chatbot
  end

  def self.with_chatbot_disabled
    disable_chatbot
    yield
    enable_chatbot
  end

  def self.privacy_policy_enabled?
    !ENV['VIKYAPP_PRIVACY_POLICY_URL'].blank? && !ENV['VIKYAPP_TERMS_OF_USE_URL'].blank?
  end

  def self.matomo_enabled?
    (
      ENV['VIKYAPP_MATOMO_ENABLED'] == 'true' &&
      !ENV['VIKYAPP_MATOMO_TRACKER_URL'].blank? &&
      !ENV['VIKYAPP_MATOMO_SITE_ID'].blank?
    )
  end
end
