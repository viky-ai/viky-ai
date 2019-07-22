module Feature

  def self.user_registration_enabled?
    ENV['VIKYAPP_USER_REGISTRATION'] == 'true'
  end

  def self.enable_user_registration
    ENV['VIKYAPP_USER_REGISTRATION'] = 'true'
  end

  def self.disable_user_registration
    ENV['VIKYAPP_USER_REGISTRATION'] = 'false'
  end
  
  def self.quota_enabled?
    ENV['VIKYAPP_QUOTA'] == 'true'
  end

  def self.enable_quota
    ENV['VIKYAPP_QUOTA'] = 'true'
  end

  def self.disable_quota
    ENV['VIKYAPP_QUOTA'] = 'false'
  end
  
end
