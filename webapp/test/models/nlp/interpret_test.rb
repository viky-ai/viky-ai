require 'test_helper'
require 'model_test_helper'

class InterpretTest < ActiveSupport::TestCase

  test "validation on now" do
    weather = agents(:weather)
    interpret = Nlp::Interpret.new(
      agents: [weather],
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
      agents: [weather],
      sentence: 'hello',
      format: 'json'
    )
    endpoint = ENV.fetch('VIKYAPP_NLP_URL') { 'http://localhost:9345' }
    assert_equal endpoint, interpret.endpoint
  end


  test 'Validate interpret URL' do
    weather = agents(:weather)
    interpret = Nlp::Interpret.new(
      agents: [weather],
      sentence: 'hello',
      format: 'json'
    )
    assert_equal "#{interpret.endpoint}/interpret/", interpret.url
  end

  test 'Request on agent id and all its dependencies' do
    weather = agents(:weather)
    terminator = agents(:terminator)
    assert AgentArc.create(source: weather, target: terminator)
    force_reset_model_cache(weather)
    interpret = Nlp::Interpret.new(
      agents: [weather],
      sentence: 'hello',
      format: 'json'
    )
    assert_equal [weather.id, terminator.id], interpret.packages
  end


  test 'Test a long interpretation sentence' do
    weather = agents(:weather)
    interpret = Nlp::Interpret.new(
      agents: [weather],
      sentence: 'a' * 1024 * 8 + 'oops',
      format: 'json'
    )
    assert interpret.invalid?
    assert_equal ["Sentence (8.004 KB) is too long (maximum is 8 KB)"], interpret.errors.full_messages
  end


  test 'with multiple agents' do
    weather = agents(:weather)
    terminator = agents(:terminator)
    assert AgentArc.create(source: weather, target: terminator)

    force_reset_model_cache(weather)
    interpret = Nlp::Interpret.new(
      agents: [weather, agents(:weather_confirmed)],
      sentence: 'Hello',
      format: 'json'
    )
    assert interpret.valid?

    assert_equal 2, interpret.agents.size
    assert interpret.agents.collect(&:id).include? weather.id
    assert interpret.agents.collect(&:id).include? agents(:weather_confirmed).id

    assert interpret.packages.include? weather.id
    assert interpret.packages.include? terminator.id
    assert interpret.packages.include? agents(:weather_confirmed).id
    assert_equal 3, interpret.packages.size
  end
end
