require 'test_helper'

class InterpretTest < ActiveSupport::TestCase

  test "validation on ownername & agentname" do
    interpret = Nlp::Interpret.new(
      ownername: 'missing user',
      agentname: 'missing agent'
    )
    assert interpret.invalid?
    assert interpret.errors.full_messages.include? "Ownername is incorrect, 'missing user' user doesn't exist"
    assert interpret.errors.full_messages.include? "Agentname is incorrect, 'missing user' user doesn't own 'missing agent' agent"


    interpret = Nlp::Interpret.new(
      ownername: 'admin',
      agentname: 'missing agent'
    )
    assert interpret.invalid?
    assert interpret.errors.full_messages.include? "Agentname is incorrect, 'admin' user doesn't own 'missing agent' agent"


    interpret = Nlp::Interpret.new(
      ownername: 'admin',
      agentname: 'weather'
    )
    assert interpret.invalid?
    assert_equal 5, interpret.errors.full_messages.count
  end


  test "validation on agent token" do
    interpret = Nlp::Interpret.new(
      ownername: 'admin',
      agentname: 'weather',
      agent_token: 'bad token',
      sentence: 'hello',
      format: 'json',
      now: '2017-12-05T10:17:25+01:00'
    )
    assert interpret.invalid?
    assert_equal ["Agent token is not valid"], interpret.errors.full_messages

    interpret = Nlp::Interpret.new(
      ownername: 'admin',
      agentname: 'weather',
      agent_token: '52f6cb86b23d8d2a0df9c6808d73ce05',
      sentence: 'hello',
      format: 'json',
      now: '2017-12-05T10:17:25+01:00'
    )
    assert interpret.valid?
  end


  test "validation on now" do
    weather = agents(:weather)
    interpret = Nlp::Interpret.new(
      ownername: weather.owner.username,
      agentname: weather.agentname,
      agent_token: weather.api_token,
      sentence: 'hello',
      format: 'json'
    )
    assert interpret.valid?

    interpret.now = "2017-12-05T15:23:17+98:00"
    assert interpret.invalid?
    expected = ["Now is incorrect, please use ISO 8601 format, i.e. 2017-12-05T15:23:17+01:00"]
    assert_equal expected, interpret.errors.full_messages

    interpret.now = "2017-12-05T15:23:17+01:00"
    assert interpret.valid?
  end


  test 'Validate endpoint' do
    weather = agents(:weather)
    interpret = Nlp::Interpret.new(
      ownername: weather.owner.username,
      agentname: weather.agentname,
      agent_token: weather.api_token,
      sentence: 'hello',
      format: 'json'
    )
    endpoint = ENV.fetch('VIKYAPP_NLP_URL') { 'http://localhost:9345' }
    assert_equal endpoint, interpret.endpoint
  end


  test 'Validate interpret URL' do
    weather = agents(:weather)
    interpret = Nlp::Interpret.new(
      ownername: weather.owner.username,
      agentname: weather.agentname,
      agent_token: weather.api_token,
      sentence: 'hello',
      format: 'json'
    )
    assert_equal "#{interpret.endpoint}/interpret/", interpret.url
  end


  test 'Request on agent id and all its dependencies' do
    weather = agents(:weather)
    terminator = agents(:terminator)
    assert AgentArc.create(source: weather, target: terminator)

    interpret = Nlp::Interpret.new(
      ownername: weather.owner.username,
      agentname: weather.agentname,
      agent_token: weather.api_token,
      sentence: 'hello',
      format: 'json'
    )
    assert_equal [weather.id, terminator.id], interpret.packages
  end

  test 'Test a long interpretation sentence' do
    weather = agents(:weather)
    interpret = Nlp::Interpret.new(
      ownername: weather.owner.username,
      agentname: weather.agentname,
      agent_token: weather.api_token,
      sentence: 'hello' * 2000,
      format: 'json'
    )
    assert interpret.invalid?
    assert_equal ['Sentence is too long'], interpret.errors.full_messages
  end
end
