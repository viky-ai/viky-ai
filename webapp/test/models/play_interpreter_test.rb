require 'test_helper'

class PlayInterpreterTest < ActiveSupport::TestCase

  test 'basic presence validations' do
    pi = PlayInterpreter.new
    assert_not pi.valid?
    expected = [
      "Text can't be blank"
    ]
    assert_equal expected, pi.errors.full_messages

    # Validation set_defaults
    assert_equal [], pi.available_agents
    assert_equal [], pi.agent_ids
    assert_nil pi.result
    assert_equal 'low', pi.spellchecking
    assert_equal '*', pi.language
  end


  test 'Text size validation' do
    pi = PlayInterpreter.new(
      text: "A"*1024*8 + "B"
    )
    assert_not pi.valid?

    expected = ["Text (8.001 KB) is too long (maximum is 8 KB)"]
    assert_equal expected, pi.errors.full_messages

    pi.text = "A"*1024*8
    assert pi.valid?
  end


  test 'available_agents and agent_ids validation' do
    pi = PlayInterpreter.new(
      text: "Hello",
      agent_ids: [1, 2]
    )
    assert pi.valid?
    assert_equal [], pi.available_agents
    assert_equal [], pi.agent_ids

    pi = PlayInterpreter.new(
      text: "Hello",
      agent_ids: [1, 2, agents(:terminator).id]
    )
    assert pi.valid?
    assert_equal [], pi.agent_ids

    pi = PlayInterpreter.new(
      text: "Hello",
      agent_ids: [1, 2, agents(:terminator).id]
    )
    pi.available_agents = [agents(:terminator)]
    assert pi.valid?
    assert_equal [agents(:terminator).id], pi.agent_ids
  end

end
