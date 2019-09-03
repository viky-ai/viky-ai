require_relative 'boot'

require 'rails/all'
require 'csv'
require 'rack/throttle'
require 'redis'

# Require the gems listed in Gemfile, including any gems
# you've limited to :test, :development, or :production.
Bundler.require(*Rails.groups)


module Webapp
  class Application < Rails::Application
    # Initialize configuration defaults for originally generated Rails version.
    config.load_defaults 5.1

    # SMTP or POSTMARK
    smtp_enabled = ENV.fetch("SMTP_ENABLED") { false }
    if smtp_enabled == "true"
      config.action_mailer.delivery_method = :smtp
      config.action_mailer.smtp_settings = {
        address:              ENV.fetch("SMTP_ADDRESS") { "localhost" },
        port:                 (ENV.fetch("SMTP_PORT") { 25 }).to_i
      }
      unless (ENV.fetch("SMTP_USER_NAME") { nil }).blank?
        config.action_mailer.smtp_settings[:user_name] = ENV.fetch("SMTP_USER_NAME")
      end
      unless (ENV.fetch("SMTP_PASSWORD") { nil }).blank?
        config.action_mailer.smtp_settings[:password] = ENV.fetch("SMTP_PASSWORD")
      end
      unless (ENV.fetch("SMTP_AUTHENTICATION") { nil }).blank?
        config.action_mailer.smtp_settings[:authentication] = ENV.fetch("SMTP_AUTHENTICATION").to_sym
      end
      if ENV.fetch("SMTP_ENABLE_STARTTLS_AUTO") { true } == "false"
        config.action_mailer.smtp_settings[:enable_starttls_auto] = false
      end
    else
      config.action_mailer.delivery_method = :postmark
      config.action_mailer.postmark_settings = {
        api_token: ENV.fetch("POSTMARK_TOKEN") { "***REMOVED***" }
      }
    end

    config.time_zone = 'Paris'

    # Settings in config/environments/* take precedence over those specified here.
    # Application configuration should go into files in config/initializers
    # -- all .rb files in that directory are automatically loaded.

    # Feature switch
    require "#{config.root}/lib/feature.rb"

    # Ping Pong Bot (Sinatra App)
    require "#{config.root}/lib/ping_pong_bot/app.rb"

    # Adding throttling
    require "#{config.root}/config/initializers/rules-custom.rb"
    
    redis = Redis.new(url: ENV.fetch('VIKYAPP_REDIS_RACK_THROTTLE') { "redis://localhost:6379/4/#{Rails.env}" })

    config.middleware.use Rack::Throttle::RulesCustom, cache: redis, key_prefix: :throttle, rules: Rack::Throttle::day_rule, time_window: :day
    config.middleware.use Rack::Throttle::RulesCustom, cache: redis, key_prefix: :throttle, rules: Rack::Throttle::hour_rule, time_window: :hour 
    config.middleware.use Rack::Throttle::RulesCustom, cache: redis, key_prefix: :throttle, rules: Rack::Throttle::minute_rule, time_window: :minute
    config.middleware.use Rack::Throttle::RulesCustom, cache: redis, key_prefix: :throttle, rules: Rack::Throttle::second_rule, time_window: :second, default: 5
    
    require "#{config.root}/app/middlewares/health_check.rb"
    config.middleware.insert_after Rails::Rack::Logger, HealthCheck

    config.active_job.queue_adapter     = :sidekiq
    config.active_job.queue_name_prefix = "webapp"
    config.active_job.queue_name_delimiter = "_"

    config.exceptions_app = self.routes

    config.active_record.schema_format = :sql
  end
end
