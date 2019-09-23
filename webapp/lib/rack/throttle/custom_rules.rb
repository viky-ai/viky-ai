module Rack
  module Throttle
    class CustomRules < Rules

      protected

      def client_identifier_for_rule(request, rule)
        api_token = request.params["agent_token"] || request.get_header("HTTP_AGENT_TOKEN")
        agent = Agent.find_by api_token: api_token

        if agent.nil?
          "#{ip(request)}_#{rule[:method]}_#{rule[:path]}"
        else
          "#{agent.owner_id}_#{rule[:method]}_#{rule[:path]}"
        end
      end

      def allowed?(request)
        return true unless Feature.quota_enabled?

        api_token = request.params["agent_token"] || request.get_header("HTTP_AGENT_TOKEN")

        unless api_token.blank?
          agent = Agent.find_by_api_token(api_token)
          user = User.find(agent.owner_id) unless agent.nil?
        end

        if user&.ignore_quota
          allowed = true
        else
          allowed = super(request)
        end

        unless allowed
          if agent
            log = InterpretRequestLog.new(
              timestamp: Time.now.iso8601(3),
              agents: [agent]
            )
            log.with_response(503, { }).save
          end
        end

        allowed
      end
    end

  end
end
