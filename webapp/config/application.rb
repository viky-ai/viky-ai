require_relative 'boot'

require 'rails/all'

# Require the gems listed in Gemfile, including any gems
# you've limited to :test, :development, or :production.
Bundler.require(*Rails.groups)

module Webapp
  class Application < Rails::Application
    # Initialize configuration defaults for originally generated Rails version.
    config.load_defaults 5.1


    config.action_mailer.delivery_method = :postmark
    config.action_mailer.postmark_settings = {
      api_token: ENV.fetch("POSTMARK_TOKEN") { "***REMOVED***" }
    }

    config.time_zone = 'Paris'

    # Settings in config/environments/* take precedence over those specified here.
    # Application configuration should go into files in config/initializers
    # -- all .rb files in that directory are automatically loaded.

    # Feature switch
    require "#{config.root}/lib/feature.rb"

    require "#{config.root}/app/middlewares/health_check.rb"
    config.middleware.insert_after Rails::Rack::Logger, HealthCheck

    config.active_job.queue_adapter     = :sidekiq
    config.active_job.queue_name_prefix = "webapp"
    config.active_job.queue_name_delimiter = "_"

    config.exceptions_app = self.routes
  end
end
