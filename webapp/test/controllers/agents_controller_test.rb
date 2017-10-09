require 'test_helper'

class AgentsControllerTest < ActionDispatch::IntegrationTest

  test "agent routes" do
    user_id  = 'admin'
    agent_id = 'weather'

    # Show
    assert_routing({
      method: 'get',
      path: "#{user_id}/#{agent_id}"
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
      path: "#{user_id}/new"
    },
    {
      controller: 'agents',
      action: 'new',
      user_id: user_id
    })

    # Create
    assert_routing({
      method: 'post',
      path: "#{user_id}"
    },
    {
      controller: 'agents',
      action: 'create',
      user_id: user_id
    })

    # Edit
    assert_routing({
      method: 'get',
      path: "#{user_id}/#{agent_id}/edit"
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
      path: "#{user_id}/#{agent_id}"
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
      path: "#{user_id}/#{agent_id}"
    },
    {
      controller: 'agents',
      action: 'destroy',
      user_id: user_id,
      id: agent_id
    })

  end

end
