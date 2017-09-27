module Feature

  def self.user_registration_enabled?
    ENV['VOQALAPP_USER_REGISTRATION'] == 'true'
  end

  def self.enable_user_registration
    ENV['VOQALAPP_USER_REGISTRATION'] = 'true'
  end

  def self.disable_user_registration
    ENV['VOQALAPP_USER_REGISTRATION'] = 'false'
  end
end
