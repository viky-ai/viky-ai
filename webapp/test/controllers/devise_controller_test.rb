require 'test_helper'

class DeviseControllerTest < ActionDispatch::IntegrationTest

  test "user edit is disable" do
    sign_in users(:admin)
    error = assert_raises ActionController::RoutingError do
      get edit_user_registration_path
    end
    assert_equal "Not Found", error.message
  end

  test "user update is disable" do
    sign_in users(:admin)
    error = assert_raises ActionController::RoutingError do
      put user_registration_path
    end
    assert_equal "Not Found", error.message
  end

  test "user destroy is disable" do
    sign_in users(:admin)
    error = assert_raises ActionController::RoutingError do
      delete user_registration_path
    end
    assert_equal "Not Found", error.message
  end

end
