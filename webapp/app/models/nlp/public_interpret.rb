module Nlp::PublicInterpret

  def self.request_public_api(request_params, agent)
    req = Rack::Request.new(
      'rack.input' => {},
      'REQUEST_METHOD' => 'GET'
    )
    req.path_info = "/api/v1/agents/#{agent.owner.username}/#{agent.agentname}/interpret.json"

    params = {
      agent_token: agent.api_token,
      sentence: request_params[:sentence],
      language: request_params[:language],
      spellchecking: request_params[:spellchecking],
      now: request_params[:now],
      context: {
        client_type: request_params[:client_type]
      }
    }
    params[:context][:user_id] = request_params[:user_id] if request_params[:user_id].present?
    params[:verbose] = request_params[:verbose] if request_params[:verbose] == 'true'
    params.each { |k, v| req.update_param(k, v) }
    base_url = ENV.fetch('VIKYAPP_BASEURL') { 'http://localhost:3000' }
    path = base_url + req.path_info + '?' + params.to_query

    response = Rails.application.call(req.env)
    status, _, body = response

    {
      path: path,
      status: status,
      body: status == 200 ? body.first : body.body
    }
  end
end
