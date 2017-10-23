require 'test_helper'

class NlsControllerTest < ActionDispatch::IntegrationTest

  test "interpret routes" do
    user_id  = 'admin'
    agent_id = 'weather'

    # Interpret
    assert_routing({
      method: 'get',
      path: "api/v1/agents/#{user_id}/#{agent_id}/interpret"
    },
    {
      format: :json,
      controller: 'api/v1/nls',
      action: 'interpret',
      user_id: user_id,
      id: agent_id
    })
  end

end
