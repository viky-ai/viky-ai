require 'test_helper'

class ErrorsControllerTest < ActionDispatch::IntegrationTest
  test "404 Error" do
    assert_raises(ActiveRecord::RecordNotFound) do
      get "/api/v1/agents/unknown_username/unknown_agent/interpret.json",
        params: {sentence: "test", agent_token: 'unknown agent api token'}
    end
  end
end
