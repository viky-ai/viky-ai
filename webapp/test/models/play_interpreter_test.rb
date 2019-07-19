require 'test_helper'

class PlayInterpreterTest < ActiveSupport::TestCase

  test 'basic presence validations' do
    pi = PlayInterpreter.new
    assert_not pi.valid?
    expected = [
      "Ownername can't be blank",
      "Agentname can't be blank",
      "Text can't be blank"
    ]
    assert_equal expected, pi.errors.full_messages
    assert_equal [], pi.agents
    assert_equal Hash.new, pi.results
  end


  test 'Text size validation' do
    pi = PlayInterpreter.new(
      ownername: agents(:weather).owner.username,
      agentname: agents(:weather).agentname,
      text: "A"*1024*8 + "B"
    )
    assert_not pi.valid?

    expected = ["Text (8.001 KB) is too long (maximum is 8 KB)"]
    assert_equal expected, pi.errors.full_messages

    pi.text = "A"*1024*8
    assert pi.valid?
  end

end
