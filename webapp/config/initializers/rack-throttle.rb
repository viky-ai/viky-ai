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
    end

    # Limitations rules

    def self.rules 
      [
        { method: "GET", path: "/assets/.*", limit: 300 },          # assets requests
        {method: "GET", path: "/api_internal/.*", limit: 1000 },   # NLS requests
        { method: "GET", path: "/api/v1/.*/test/interpret.json", limit: 10 }
      ]
    end

    def self.day_rule 
      [
        { method: "GET", path: "/api/v1/.*/test/interpret.json", limit: 15000 }
      ]
    end

    def self.hour_rule 
      [
        { method: "GET", path: "/api/v1/.*/test/interpret.json", limit: 1000 }
      ]
    end

    def self.minute_rule 
      [
        { method: "GET", path: "/api/v1/.*/test/interpret.json", limit: 20 }
      ]
    end

  end
end