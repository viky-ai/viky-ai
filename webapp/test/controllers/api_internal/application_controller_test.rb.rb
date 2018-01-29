require 'test_helper'

class ApplicationControllerTest < ActionDispatch::IntegrationTest

  test "A request without Access-Token header" do

    get "/api_internal/packages"
    assert_equal '401', response.code
    assert_equal "Access denied: wrong token.", JSON.parse(response.body)["message"]

    get "/api_internal/packages/fba88ff8-8238-5007-b3d8-b88fd504f94c.xml"
    assert_equal '401', response.code
    assert_equal "Access denied: wrong token.", JSON.parse(response.body)["message"]
  end

  test "A request without json format is not accepted" do

    get "/api_internal/packages", headers: { "Access-Token" => ENV["VIKYAPP_INTERNAL_API_TOKEN"] }
    assert_equal '406', response.code
    assert_equal "Format not acceptable. JSON only is accepted.", JSON.parse(response.body)["message"]

    get "/api_internal/packages/fba88ff8-8238-5007-b3d8-b88fd504f94c.xml", headers: { "Access-Token" => ENV["VIKYAPP_INTERNAL_API_TOKEN"] }
    assert_equal '406', response.code
    assert_equal "Format not acceptable. JSON only is accepted.", JSON.parse(response.body)["message"]
  end

end
