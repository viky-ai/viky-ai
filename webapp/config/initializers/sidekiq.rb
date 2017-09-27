Sidekiq.configure_server do |config|
  config.average_scheduled_poll_interval = 2
  config.redis = {
    url: ENV['VOQALAPP_ACTIVEJOB_REDIS_URL'] || 'redis://localhost:7372/1'
  }
end

Sidekiq.configure_client do |config|
  config.redis = {
    url: ENV['VOQALAPP_ACTIVEJOB_REDIS_URL'] || 'redis://localhost:7372/1'
  }
end
