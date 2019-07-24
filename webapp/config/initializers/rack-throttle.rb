module Rack
  module Throttle
    class RulesCustom < Rules
      
      protected
      def client_identifier_for_rule(request, rule)
        agent = Agent.find_by api_token: request.params["agent_token"]

        if agent.nil?
          "#{ip(request)}_#{rule[:method]}_#{rule[:path]}"
        else
          "#{agent.owner_id}_#{rule[:method]}_#{rule[:path]}"
        end
      end

      def allowed?(request)
        

        if Feature.rack_throttle_enabled?
          if !Feature.rack_throttle_limit_day_enabled? && @options[:time_window] == :day
            allowed = true
          elsif !Feature.rack_throttle_limit_hour_enabled? && @options[:time_window] == :hour
            allowed = true
          elsif !Feature.rack_throttle_limit_minute_enabled? && @options[:time_window] == :minute
            allowed = true
          elsif !Feature.rack_throttle_limit_second_enabled? && @options[:time_window] == :second
            allowed = true
          else
            return true if whitelisted?(request)
            count = cache_get(key = cache_key(request)).to_i + 1 rescue 1
            allowed = count <= max_per_window(request).to_i
            begin
              cache_set(key, count)
              allowed
            rescue => e
              allowed = true
            end
          end
        else 
          allowed = true
        end
        allowed
      end
    end

    def self.day_limit 
      Integer(ENV.fetch("VIKYAPP_RACK_THROTTLE_LIMIT_DAY") { 15_000 })
    end

    def self.hour_limit 
      Integer(ENV.fetch("VIKYAPP_RACK_THROTTLE_LIMIT_HOUR") { 1_000 })
    end

    def self.minute_limit 
      Integer(ENV.fetch("VIKYAPP_RACK_THROTTLE_LIMIT_MINUTE") { 20 })
    end

    def self.second_limit 
      Integer(ENV.fetch("VIKYAPP_RACK_THROTTLE_LIMIT_SECOND") { 10 })
    end

    # Limitations rules

    def self.second_rule 
      [
        { method: "GET", path: "/assets/.*", limit: 300 },          # assets requests
        { method: "GET", path: "/api_internal/.*", limit: 1000 },   # NLS requests
        { method: "GET", path: "/api/v1/agents/.*/interpret.json", limit: self.second_limit }
      ]
    end

    def self.day_rule 
      [
        { method: "GET", path: "/api/v1/agents/.*/interpret.json", limit: self.day_limit }
      ]
    end

    def self.hour_rule 
      [
        { method: "GET", path: "/api/v1/agents/.*/interpret.json", limit: self.hour_limit }
      ]
    end

    def self.minute_rule 
      [
        { method: "GET", path: "/api/v1/agents/.*/interpret.json", limit: self.minute_limit }
      ]
    end

  end
end