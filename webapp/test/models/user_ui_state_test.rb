require 'test_helper'

class UserUiStateTest < ActiveSupport::TestCase

  test "non existing destination" do
    user = users(:edit_on_agent_weather)
    agent = agents(:weather)
    user.ui_state = {"last_agent_move"=>"414211c7-d6cb-44f1-8b0c-bb6c192b92b6"}
    user.save
    user_state = UserUiState.new(user)
    assert_nil user_state.last_destination_agent(agent)
  end

  test "existing destination" do
    user = users(:admin)
    agent = agents(:weather)
    destination_agent_id = agents(:terminator).id
    user.ui_state = {"last_agent_move"=>destination_agent_id}
    user.save
    user_state = UserUiState.new(user)
    assert_not_nil user_state.last_destination_agent(agent)
  end

end
