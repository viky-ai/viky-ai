require 'test_helper'

class ApiInternalTest < ActionDispatch::IntegrationTest

  test "Api internal get all packages" do
    get "/api_internal/packages.json", headers: { "Access-Token" => ENV["VIKYAPP_INTERNAL_API_TOKEN"] }

    assert_equal '200', response.code

    expected = ["930025d1-cfd0-5f27-8cb1-a0aecd1309fc","794f5279-8ed5-5563-9229-3d2573f23051","fba88ff8-8238-5007-b3d8-b88fd504f94c"]
    assert_equal expected, JSON.parse(response.body)
  end

  test "Api internal get one package" do
    get "/api_internal/packages/fba88ff8-8238-5007-b3d8-b88fd504f94c.json", headers: { "Access-Token" => ENV["VIKYAPP_INTERNAL_API_TOKEN"] }

    assert_equal '200', response.code

    expected = {
      "id" => "fba88ff8-8238-5007-b3d8-b88fd504f94c",
      "slug" => "confirmed/weather",
      "interpretations" => []
    }
    assert_equal expected, JSON.parse(response.body)
  end

end
