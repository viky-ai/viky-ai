module Feature

  # User registration
  def self.user_registration_enabled?
    ENV['VIKYAPP_USER_REGISTRATION'] == 'true'
  end

  def self.enable_user_registration
    ENV['VIKYAPP_USER_REGISTRATION'] = 'true'
  end

  def self.disable_user_registration
    ENV['VIKYAPP_USER_REGISTRATION'] = 'false'
  end

  # Quota
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

end
