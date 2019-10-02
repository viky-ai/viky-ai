require 'rack/throttle'
require "#{Rails.root}/lib/rack/throttle/custom_rules.rb"

module Quota

  def self.cache
    Redis.new(
      url: ENV.fetch('VIKYAPP_QUOTA_REDIS_URL') { "redis://localhost:6379/4/#{Rails.env}" }
    )
  end

  def self.reset_cache
    self.cache.flushdb
  end

  def self.throttle_rules_for(period)
    interpret_path = "/api/v1/agents/.*/interpret.json"
    rules = {
      second: [{ method: "GET", path: interpret_path, limit: self.max_interpret_requests_per_second }],
      day:    [{ method: "GET", path: interpret_path, limit: self.max_interpret_requests_per_day }]
    }
    rules[period]
  end

  def self.max_interpret_requests_per_second
    ENV.fetch("VIKYAPP_QUOTA_INTERPRET_PER_SECOND") { 10 }.to_i
  end

  def self.max_interpret_requests_per_day
    ENV.fetch("VIKYAPP_QUOTA_INTERPRET_PER_DAY") { 15_000 }.to_i
  end

  def self.expressions_limit
    ENV.fetch('VIKYAPP_QUOTA_EXPRESSION') { 500_000 }.to_i
  end
end

[:second, :day].each do |period|
  Rails.configuration.middleware.use(
    Rack::Throttle::CustomRules,
    cache: Quota.cache,
    key_prefix: :throttle,
    rules: Quota.throttle_rules_for(period),
    time_window: period
  )
end
