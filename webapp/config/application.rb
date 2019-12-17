require_relative 'boot'

require "rails"
require "active_model/railtie"
require "active_job/railtie"
require "active_record/railtie"
require "action_controller/railtie"
require "action_mailer/railtie"
require "action_view/railtie"
require "action_cable/engine"
require "sprockets/railtie"
require "rails/test_unit/railtie"

require 'csv'

# Require the gems listed in Gemfile, including any gems
# you've limited to :test, :development, or :production.
Bundler.require(*Rails.groups)

module Webapp
  class Application < Rails::Application
    # Initialize configuration defaults for originally generated Rails version.
    config.load_defaults 6.0

    # SMTP or POSTMARK
    smtp_enabled = ENV.fetch("SMTP_ENABLED") { false }
    postmark_enabled = ENV.fetch("POSTMARK_TOKEN") { false }
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
    elsif postmark_enabled
      config.action_mailer.delivery_method = :postmark
      config.action_mailer.postmark_settings = {
        api_token: ENV.fetch("POSTMARK_TOKEN")
      }
    end

    config.time_zone = 'Paris'

    # Settings in config/environments/* take precedence over those specified here.
    # Application configuration should go into files in config/initializers
    # -- all .rb files in that directory are automatically loaded.

    # Feature switch
    require "#{config.root}/lib/feature.rb"

    # load extensions
    Dir[File.join(Rails.root, 'lib', 'extensions', '*.rb')].each {|l| require l }

    # Rack health_check middleware
    require "#{config.root}/app/middlewares/health_check.rb"
    config.middleware.insert_after Rails::Rack::Logger, HealthCheck

    # Use Sidekiq
    config.active_job.queue_adapter     = :sidekiq
    config.active_job.queue_name_prefix = "webapp"
    config.active_job.queue_name_delimiter = "_"

    # Others
    config.exceptions_app = self.routes
    config.active_record.schema_format = :sql
  end
end
