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

  # RACK THROTTLE
  
  def self.rack_throttle_enabled?
    ENV['VIKYAPP_RACK_THROTTLE_ENABLED'] == 'true'
  end

  def self.enable_rack_throttle
    ENV['VIKYAPP_RACK_THROTTLE_ENABLED'] = 'true'
  end

  def self.disable_rack_throttle
    ENV['VIKYAPP_RACK_THROTTLE_ENABLED'] = 'false'
  end
  
  # Second
  def self.rack_throttle_limit_second_disabled?
    ENV['VIKYAPP_RACK_THROTTLE_LIMIT_SECOND_DISABLED'] == 'true'
  end

  def self.enable_rack_throttle_limit_second
    ENV['VIKYAPP_RACK_THROTTLE_LIMIT_SECOND_DISABLED'] = 'false'
  end

  def self.disable_rack_throttle_limit_second
    ENV['VIKYAPP_RACK_THROTTLE_LIMIT_SECOND_DISABLED'] = 'true'
  end
  
  # Minute
  def self.rack_throttle_limit_minute_disabled?
    ENV['VIKYAPP_RACK_THROTTLE_LIMIT_MINUTE_DISABLED'] == 'true'
  end

  def self.enable_rack_throttle_limit_minute
    ENV['VIKYAPP_RACK_THROTTLE_LIMIT_MINUTE_DISABLED'] = 'false'
  end

  def self.disable_rack_throttle_limit_minute
    ENV['VIKYAPP_RACK_THROTTLE_LIMIT_MINUTE_DISABLED'] = 'true'
  end
  
  # Hour
  def self.rack_throttle_limit_hour_disabled?
    ENV['VIKYAPP_RACK_THROTTLE_LIMIT_HOUR_DISABLED'] == 'true'
  end

  def self.enable_rack_throttle_limit_hour
    ENV['VIKYAPP_RACK_THROTTLE_LIMIT_HOUR_DISABLED'] = 'false'
  end

  def self.disable_rack_throttle_limit_hour
    ENV['VIKYAPP_RACK_THROTTLE_LIMIT_HOUR_DISABLED'] = 'true'
  end
  
  # Day
  def self.rack_throttle_limit_day_disabled?
    ENV['VIKYAPP_RACK_THROTTLE_LIMIT_DAY_DISABLED'] == 'true'
  end

  def self.enable_rack_throttle_limit_day
    ENV['VIKYAPP_RACK_THROTTLE_LIMIT_DAY_DISABLED'] = 'false'
  end

  def self.disable_rack_throttle_limit_day
    ENV['VIKYAPP_RACK_THROTTLE_LIMIT_DAY_DISABLED'] = 'true'
  end
  
end
