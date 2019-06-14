require 'test_helper'

class AgentsControllerTest < ActionDispatch::IntegrationTest

  test "agent routes" do
    user_id  = 'admin'
    agent_id = 'weather'

    # Show
    assert_routing({
      method: 'get',
      path: "agents/#{user_id}/#{agent_id}"
    },
    {
      controller: 'agents',
      action: 'show',
      user_id: user_id,
      id: agent_id
    })

    # new
    assert_routing({
      method: 'get',
      path: "agents/#{user_id}/new"
    },
    {
      controller: 'agents',
      action: 'new',
      user_id: user_id
    })

    # Create
    assert_routing({
      method: 'post',
      path: "agents/#{user_id}"
    },
    {
      controller: 'agents',
      action: 'create',
      user_id: user_id
    })

    # Edit
    assert_routing({
      method: 'get',
      path: "agents/#{user_id}/#{agent_id}/edit"
    },
    {
      controller: 'agents',
      action: 'edit',
      user_id: user_id,
      id: agent_id
    })

    # update
    assert_routing({
      method: 'patch',
      path: "agents/#{user_id}/#{agent_id}"
    },
    {
      controller: 'agents',
      action: 'update',
      user_id: user_id,
      id: agent_id
    })

    # destroy
    assert_routing({
      method: 'delete',
      path: "agents/#{user_id}/#{agent_id}"
    },
    {
      controller: 'agents',
      action: 'destroy',
      user_id: user_id,
      id: agent_id
    })

    # Generate token
    assert_routing({
      method: 'get',
      path: "agents/#{user_id}/#{agent_id}/generate_token"
    },
    {
      controller: 'agents',
      action: 'generate_token',
      user_id: user_id,
      id: agent_id
    })
  end


  #
  # Show access
  #
  test "Show access: User is owner" do
    sign_in users(:admin)

    get user_agent_url(users(:admin), agents(:weather))
    assert_response :success
    assert_nil flash[:alert]
  end

  test "Show access: User has no relation to agent" do
    sign_in users(:admin)

    get user_agent_url(users(:confirmed), agents(:weather_confirmed))
    assert_redirected_to agents_url
    assert_equal "Unauthorized operation.", flash[:alert]
  end

  test 'Show access: Public agent' do
    sign_in users(:admin)
    agent_public = agents(:weather_confirmed)
    agent_public.visibility = 'is_public'
    assert agent_public.save

    get user_agent_url(users(:confirmed), agent_public)
    assert_response :success
    assert_nil flash[:alert]
  end

  test "Show access: Collaborator (show)" do
    sign_in users(:show_on_agent_weather)

    get user_agent_url(users(:admin), agents(:weather))
    assert_response :success
    assert_nil flash[:alert]
  end

  test "Show access: Collaborator (edit)" do
    sign_in users(:edit_on_agent_weather)

    get user_agent_url(users(:admin), agents(:weather))
    assert_response :success
    assert_nil flash[:alert]
  end


  #
  # Edit access
  #
  test "Edit access: User is owner" do
    sign_in users(:admin)

    get edit_user_agent_url(users(:admin), agents(:weather))
    assert_response :success
    assert_nil flash[:alert]
  end

  test "Edit access: User has no relation to agent" do
    sign_in users(:admin)

    get edit_user_agent_url(users(:confirmed), agents(:weather_confirmed))
    assert_redirected_to agents_url
    assert_equal "Unauthorized operation.", flash[:alert]
  end

  test "Edit access: Public agent" do
    sign_in users(:admin)
    agent_public = agents(:weather_confirmed)
    agent_public.visibility = 'is_public'
    assert agent_public.save

    get edit_user_agent_url(users(:confirmed), agent_public)
    assert_redirected_to agents_url()
    assert_equal "Unauthorized operation.", flash[:alert]
  end

  test "Edit access: Collaborator (show)" do
    sign_in users(:show_on_agent_weather)

    get edit_user_agent_url(users(:admin), agents(:weather))
    assert_redirected_to agents_url()
    assert_equal "Unauthorized operation.", flash[:alert]
  end

  test "Edit access: Collaborator (edit)" do
    sign_in users(:edit_on_agent_weather)

    get edit_user_agent_url(users(:admin), agents(:weather))
    assert_response :success
    assert_nil flash[:alert]
  end


  #
  # Update access
  #
  test "Update access: User is owner" do
    sign_in users(:admin)

    patch user_agent_url(users(:admin), agents(:weather)),
      params: {
        agent: { name: 'toto' },
        format: :json
      }
    assert_redirected_to agents_url
    assert_equal "Your agent has been successfully updated.", flash[:notice]
  end

  test "Update access: User has no relation to agent" do
    sign_in users(:admin)

    patch user_agent_url(users(:confirmed), agents(:weather_confirmed)),
      params: {
        agent: { name: 'toto' },
        format: :json
      }
    assert_response :forbidden
    assert response.body.include?("Unauthorized operation.")
  end

  test "Update access: Public agent" do
    sign_in users(:admin)
    agent_public = agents(:weather_confirmed)
    agent_public.visibility = 'is_public'
    assert agent_public.save

    patch user_agent_url(users(:confirmed), agent_public),
          params: {
            agent: { name: 'toto' },
            format: :json
          }
    assert_response :forbidden
    assert response.body.include?("Unauthorized operation.")
  end

  test "Update access: Collaborator (show)" do
    sign_in users(:show_on_agent_weather)

    patch user_agent_url(users(:admin), agents(:weather)),
      params: {
        agent: { name: 'toto' },
        format: :json
      }
    assert_response :forbidden
    assert response.body.include?("Unauthorized operation.")
  end

  test "Update access: Collaborator (edit)" do
    sign_in users(:edit_on_agent_weather)

    patch user_agent_url(users(:admin), agents(:weather)),
      params: {
        agent: { name: 'toto' },
        format: :json
      }
    assert_redirected_to agents_url
    assert_equal "Your agent has been successfully updated.", flash[:notice]
  end


  #
  # Confirm_destroy access
  #
  test "Confirm_destroy access: User is owner" do
    sign_in users(:admin)

    get confirm_destroy_user_agent_url(users(:admin), agents(:weather))
    assert_response :success
    assert_nil flash[:alert]
  end

  test "Confirm_destroy access: User has no relation to agent" do
    sign_in users(:admin)

    get confirm_destroy_user_agent_url(users(:confirmed), agents(:weather_confirmed))
    assert_redirected_to agents_url
    assert_equal "Unauthorized operation.", flash[:alert]
  end

  test "Confirm_destroy access: Public agent" do
    sign_in users(:admin)
    agent_public = agents(:weather_confirmed)
    agent_public.visibility = 'is_public'
    assert agent_public.save

    get confirm_destroy_user_agent_url(users(:confirmed), agent_public)
    assert_redirected_to agents_url
    assert_equal "Unauthorized operation.", flash[:alert]
  end

  test "Confirm_destroy access: Collaborator (show)" do
    sign_in users(:show_on_agent_weather)

    get confirm_destroy_user_agent_url(users(:admin), agents(:weather_confirmed))
    assert_redirected_to agents_url
    assert_equal "Unauthorized operation.", flash[:alert]
  end

  test "Confirm_destroy access: Collaborator (edit)" do
    sign_in users(:edit_on_agent_weather)

    get confirm_destroy_user_agent_url(users(:admin), agents(:weather_confirmed))
    assert_redirected_to agents_url
    assert_equal "Unauthorized operation.", flash[:alert]
  end


  #
  # Destroy access
  #
  test "Destroy access: User is owner" do
    sign_in users(:admin)

    delete user_agent_url(users(:admin), agents(:weather))
    assert_redirected_to agents_url
    expected = "Deletion failed: You must remove all collaborators before delete an agent."
    assert_equal expected, flash[:alert]
  end

  test "Destroy access: User has no relation to agent" do
    sign_in users(:admin)

    delete user_agent_url(users(:confirmed), agents(:weather_confirmed))
    assert_redirected_to agents_url
    assert_equal "Unauthorized operation.", flash[:alert]
  end

  test "Destroy access: Public user" do
    sign_in users(:admin)
    agent_public = agents(:weather_confirmed)
    agent_public.visibility = 'is_public'
    assert agent_public.save

    delete user_agent_url(users(:confirmed), agent_public)
    assert_redirected_to agents_url
    assert_equal "Unauthorized operation.", flash[:alert]
  end

  test "Destroy access: Collaborator (show)" do
    sign_in users(:show_on_agent_weather)

    delete user_agent_url(users(:admin), agents(:weather))
    assert_redirected_to agents_url
    assert_equal "Unauthorized operation.", flash[:alert]
  end

  test "Destroy access: Collaborator (edit)" do
    sign_in users(:edit_on_agent_weather)

    delete user_agent_url(users(:admin), agents(:weather))
    assert_redirected_to agents_url
    assert_equal "Unauthorized operation.", flash[:alert]
  end


  #
  # Confirm_transfer_ownership access
  #
  test "Confirm_transfer_ownership access: User is owner" do
    sign_in users(:admin)

    get confirm_transfer_ownership_user_agent_path(users(:admin), agents(:weather))
    assert_response :success
    assert_nil flash[:alert]
  end

  test "Confirm_transfer access: User has no relation to agent" do
    sign_in users(:admin)

    get confirm_transfer_ownership_user_agent_path(users(:confirmed), agents(:weather_confirmed))
    assert_redirected_to agents_url
    assert_equal "Unauthorized operation.", flash[:alert]
  end

  test "Confirm_transfer access: Public agent" do
    sign_in users(:admin)
    agent_public = agents(:weather_confirmed)
    agent_public.visibility = 'is_public'
    assert agent_public.save

    get confirm_transfer_ownership_user_agent_path(users(:confirmed), agent_public)
    assert_redirected_to agents_url
    assert_equal "Unauthorized operation.", flash[:alert]
  end

  test "Confirm_transfer access: Collaborator (show)" do
    sign_in users(:show_on_agent_weather)

    get confirm_transfer_ownership_user_agent_path(users(:admin), agents(:weather))
    assert_redirected_to agents_url
    assert_equal "Unauthorized operation.", flash[:alert]
  end

  test "Confirm_transfer access: Collaborator (edit)" do
    sign_in users(:edit_on_agent_weather)

    get confirm_transfer_ownership_user_agent_path(users(:admin), agents(:weather))
    assert_redirected_to agents_url
    assert_equal "Unauthorized operation.", flash[:alert]
  end


  #
  # Transfer_ownership access
  #
  test "Transfer_ownership access: User is owner" do
    sign_in users(:admin)

    post transfer_ownership_user_agent_path(users(:admin), agents(:weather).agentname),
      params: {
        users: { new_owner_id: users('locked').id },
        format: :json
      }
    assert_redirected_to agents_url
    assert_nil flash[:alert]
    assert_equal "Agent My awesome weather bot transferred to user locked", flash[:notice]
  end

  test "Transfer_ownership access: User has no relation to agent" do
    sign_in users(:admin)

    post transfer_ownership_user_agent_path(users(:confirmed), agents(:weather_confirmed).agentname),
      params: {
        users: { new_owner_id: users('locked').id },
        format: :json
      }
    assert_response :forbidden
    assert response.body.include?("Unauthorized operation.")
  end

  test "Transfer_ownership access: Public agent" do
    sign_in users(:admin)
    agent_public = agents(:weather_confirmed)
    agent_public.visibility = 'is_public'
    assert agent_public.save

    post transfer_ownership_user_agent_path(users(:confirmed), agent_public.agentname),
         params: {
           users: { new_owner_id: users('locked').id },
           format: :json
         }
    assert_response :forbidden
    assert response.body.include?("Unauthorized operation.")
  end

  test "Transfer_ownership access: Collaborator (show)" do
    sign_in users(:show_on_agent_weather)

    post transfer_ownership_user_agent_path(users(:admin), agents(:weather)),
      params: {
        users: { new_owner_id: users('locked').id },
        format: :json
      }
    assert_response :forbidden
    assert response.body.include?("Unauthorized operation.")
  end

  test "Transfer_ownership access: Collaborator (edit)" do
    sign_in users(:edit_on_agent_weather)

    post transfer_ownership_user_agent_path(users(:admin), agents(:weather)),
      params: {
        users: { new_owner_id: users('locked').id },
        format: :json
      }
    assert_response :forbidden
    assert response.body.include?("Unauthorized operation.")
  end


  #
  # Dependencies access
  #

  test "successors_graph access: Collaborator (show)" do
    sign_in users(:show_on_agent_weather)

    get successors_graph_user_agent_dependencies_path(users(:admin), agents(:weather))
    assert_response :success
    assert_nil flash[:alert]
  end

  test "successors_graph access: no access" do
    sign_in users(:confirmed)

    get successors_graph_user_agent_dependencies_path(users(:admin), agents(:weather))
    assert_redirected_to agents_url
    assert_equal "Unauthorized operation.", flash[:alert]
  end

  test "predecessors_graph access: Collaborator (show)" do
    sign_in users(:show_on_agent_weather)

    get predecessors_graph_user_agent_dependencies_path(users(:admin), agents(:weather))
    assert_response :success
    assert_nil flash[:alert]
  end

  test "predecessors_graph access: no access" do
    sign_in users(:confirmed)

    get predecessors_graph_user_agent_dependencies_path(users(:admin), agents(:weather))
    assert_redirected_to agents_url
    assert_equal "Unauthorized operation.", flash[:alert]
  end

  test "create dependency access: Collaborator (show) no acccess" do
    sign_in users(:show_on_agent_weather)

    post user_agent_dependencies_path(users(:admin), agents(:weather)),
      params: {
        id: agents(:terminator).id
      }
    assert_redirected_to agents_url
    assert_equal "Unauthorized operation.", flash[:alert]
  end

  test "create dependency access: Collaborator (edit)" do
    sign_in users(:edit_on_agent_weather)

    post user_agent_dependencies_path(users(:admin), agents(:weather)),
      params: {
        id: agents(:terminator).id
      }
    assert_redirected_to user_agent_url(users(:admin), agents(:weather))
  end

  test "Destroy dependency access: Collaborator (show) no acccess" do
    assert AgentArc.create(source: agents(:weather), target: agents(:terminator))

    sign_in users(:show_on_agent_weather)

    delete user_agent_dependency_path(users(:admin), agents(:weather), agents(:terminator).id)

    assert_redirected_to agents_url
    assert_equal "Unauthorized operation.", flash[:alert]
  end

  test "Destroy dependency access: Collaborator (edit)" do
    assert AgentArc.create(source: agents(:weather), target: agents(:terminator))

    sign_in users(:edit_on_agent_weather)

    delete user_agent_dependency_path(users(:admin), agents(:weather), agents(:terminator).id)

    assert_redirected_to user_agent_url(users(:admin), agents(:weather))
  end


  #
  # Duplicate agent
  #
  test 'Duplication allowed' do
    sign_in users(:admin)
    post duplicate_user_agent_path(users(:admin), agents(:weather))
    assert_response :success
    assert_nil flash[:alert]
  end

  test 'Duplication forbidden' do
    sign_in users(:confirmed)
    post duplicate_user_agent_path(users(:admin), agents(:weather))
    assert_redirected_to agents_url
    assert_equal "Unauthorized operation.", flash[:alert]
  end
end
