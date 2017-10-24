require 'test_helper'

class MembershipsControllerTest < ActionDispatch::IntegrationTest

  #
  # index access
  #
  test "Show access: User is owner" do
    sign_in users(:admin)

    get user_agent_memberships_url(users(:admin), agents(:weather))
    assert_response :success
    assert_nil flash[:alert]
  end

  test "Show access: User has no relation to agent" do
    sign_in users(:admin)

    get user_agent_memberships_url(users(:confirmed), agents(:weather_confirmed))
    assert_redirected_to agents_url
    assert_equal "Unauthorized operation.", flash[:alert]
  end

  test "Show access: Collaborator (show)" do
    sign_in users(:show_on_agent_weather)

    get user_agent_memberships_url(users(:admin), agents(:weather))
    assert_redirected_to agents_url
    assert_equal "Unauthorized operation.", flash[:alert]
  end

  test "Show access: Collaborator (edit)" do
    sign_in users(:edit_on_agent_weather)

    get user_agent_memberships_url(users(:admin), agents(:weather))
    assert_redirected_to agents_url
    assert_equal "Unauthorized operation.", flash[:alert]
  end

  #
  # New access
  #
  test "New access: User is owner" do
    sign_in users(:admin)

    get new_user_agent_membership_url(users(:admin), agents(:weather))
    assert_response :success
    assert_nil flash[:alert]
  end

  test "New access: User has no relation to agent" do
    sign_in users(:admin)

    get new_user_agent_membership_url(users(:confirmed), agents(:weather_confirmed))
    assert_redirected_to agents_url
    assert_equal "Unauthorized operation.", flash[:alert]
  end

  test "New access: Collaborator (show)" do
    sign_in users(:show_on_agent_weather)

    get new_user_agent_membership_url(users(:admin), agents(:weather))
    assert_redirected_to agents_url
    assert_equal "Unauthorized operation.", flash[:alert]
  end

  test "New access: Collaborator (edit)" do
    sign_in users(:edit_on_agent_weather)

    get new_user_agent_membership_url(users(:admin), agents(:weather))
    assert_redirected_to agents_url
    assert_equal "Unauthorized operation.", flash[:alert]
  end

  #
  # Create access
  #
  test "Create access: User is owner" do
    sign_in users(:admin)

    post user_agent_memberships_url(users(:admin), agents(:weather)),
      params: {
        memberships: { usernames: users(:locked).username, rights: 'show' },
        format: :json
      }
    assert_redirected_to agents_url
    assert_equal "Agent weather shared with : locked.", flash[:notice]
  end

  test "Create access: User has no relation to agent" do
    sign_in users(:admin)

    post user_agent_memberships_url(users(:confirmed), agents(:weather_confirmed)),
      params: {
        memberships: { usernames: users(:locked).username, rights: 'show' },
        format: :json
      }
    assert_response :forbidden
    assert response.body.include?("Unauthorized operation.")
  end

  test "Create access: Collaborator (show)" do
    sign_in users(:show_on_agent_weather)

    post user_agent_memberships_url(users(:admin), agents(:weather)),
      params: {
        memberships: { usernames: users(:locked).username, rights: 'show' },
        format: :json
      }
    assert_response :forbidden
    assert response.body.include?("Unauthorized operation.")
  end

  test "Create access: Collaborator (edit)" do
    sign_in users(:edit_on_agent_weather)

    post user_agent_memberships_url(users(:admin), agents(:weather)),
      params: {
        memberships: { usernames: users(:locked).username, rights: 'show' },
        format: :json
      }
    assert_response :forbidden
    assert response.body.include?("Unauthorized operation.")
  end

  #
  # Update access
  #
  test "Update access: User is owner" do
    sign_in users(:admin)

    patch user_agent_membership_url(users(:admin), agents(:weather), memberships(:four)),
      params: {
        membership: { rights: 'edit' },
        format: :js
      }
    assert_response :success
  end

  test "Update access: User has no relation to agent" do
    sign_in users(:confirmed)

    patch user_agent_membership_url(users(:admin), agents(:weather), memberships(:four)),
      params: {
        membership: { rights: 'edit' },
        format: :js
      }
    assert_redirected_to agents_url
    assert_equal "Unauthorized operation.", flash[:alert]
  end

  test "Update access: Collaborator (show)" do
    sign_in users(:show_on_agent_weather)

    patch user_agent_membership_url(users(:admin), agents(:weather), memberships(:four)),
      params: {
        membership: { rights: 'edit' },
        format: :js
      }
    assert_redirected_to agents_url
    assert_equal "Unauthorized operation.", flash[:alert]
  end

  test "Update access: Collaborator (edit)" do
    sign_in users(:edit_on_agent_weather)

    patch user_agent_membership_url(users(:admin), agents(:weather), memberships(:four)),
      params: {
        membership: { rights: 'edit' },
        format: :js
      }
    assert_redirected_to agents_url
    assert_equal "Unauthorized operation.", flash[:alert]
  end

end
