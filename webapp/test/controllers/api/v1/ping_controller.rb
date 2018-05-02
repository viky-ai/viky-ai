require 'test_helper'

class PingControllerTest < ActionDispatch::IntegrationTest

  test 'ping' do
    get "/api/v1/ping.json"
    assert_equal '200', response.code
  end

end
