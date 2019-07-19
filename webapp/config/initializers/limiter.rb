module Rack
  module Throttle
    class RulesCustom < Rules 
      protected
      def client_identifier_for_rule(request, rule)
        agent = Agent.find_by api_token: request.params["agent_token"]
        "#{agent.owner_id}_#{rule[:method]}_#{rule[:path]}"
      end
    end

    # Limitations rules

    @@rules = [
      { method: "GET", path: "/assets/.*", limit: 300 },          # assets requests
      { method: "GET", path: "/api_internal/.*", limit: 1000 },   # NLS requests
      { method: "GET", path: "/api/v1/.*/test/interpret.json", limit: 10 }
    ]

    @@day_rule = [
      { method: "GET", path: "/api/v1/.*/test/interpret.json", limit: 15000 }
    ]
    @@hour_rule = [
      { method: "GET", path: "/api/v1/.*/test/interpret.json", limit: 1000 }
    ]
    @@minute_rule = [
      { method: "GET", path: "/api/v1/.*/test/interpret.json", limit: 20 }
    ]

    def self.rules
      @@rules
    end

    def self.day_rule
      @@day_rule
    end

    def self.hour_rule
      @@hour_rule
    end

    def self.minute_rule
      @@minute_rule
    end

  end
end