require 'test_helper'

class AgentRegressionCheckTest < ActiveSupport::TestCase

  test 'Basic AgentRegressionCheck creation & agent association' do
    assert_equal 2, agents(:weather).agent_regression_checks.count

    expected_nlp = {
      interpretations: [{
        id: '1234567890',
        slug: 'admin/weather/interpretation/weather_forecast',
        name: 'weather_forecast',
        score: 1.0
      }]
    }.with_indifferent_access
    agent_regression_check = AgentRegressionCheck.new(
      sentence: 'What the weather like ?',
      language: 'fr',
      now: '2019-01-17T09:00:00+01:00',
      expected: expected_nlp,
      got: expected_nlp,
      state: 'passed',
      position: 2,
      agent: agents(:weather)
    )
    assert agent_regression_check.save
    assert_equal 3, agents(:weather).agent_regression_checks.count

    assert_equal 'What the weather like ?', agent_regression_check.sentence
    assert_equal 'fr', agent_regression_check.language
    assert_equal DateTime.parse('2019-01-17T09:00:00+01:00'), agent_regression_check.now
    assert_equal expected_nlp, agent_regression_check.expected
    assert_equal expected_nlp, agent_regression_check.got
    assert_equal 'passed', agent_regression_check.state
    assert agent_regression_check.passed?
    assert_not agent_regression_check.unknown?
    assert_not agent_regression_check.failed?
    assert_equal 2, agent_regression_check.position
  end


  test 'Agent, sentence and expected are mandatory' do
    agent_regression_check = AgentRegressionCheck.new(state: 'failed')
    assert_not agent_regression_check.save

    expected = {
      agent: ['must exist'],
      sentence: ["can't be blank"],
      expected: ["can't be blank"]
    }
    assert_equal expected, agent_regression_check.errors.messages
  end


  test 'Sentence maximum length' do
    agent_regression_check = AgentRegressionCheck.new(
      agent: agents(:weather),
      sentence: 'a' * 2051,
      expected: '{}'
    )
    assert_not agent_regression_check.save

    expected = {
      sentence: ['is too long (maximum is 2050 characters)']
    }
    assert_equal expected, agent_regression_check.errors.messages
  end

  test 'Expected maximum length' do
    agent_regression_check = AgentRegressionCheck.new(
      agent: agents(:weather),
      sentence: 'a',
      expected: {
        interpretations: 'a' * 10_001
      }.with_indifferent_access
    )
    assert_not agent_regression_check.save

    expected = {
      expected_as_str: ['is too long (maximum is 10000 characters)']
    }
    assert_equal expected, agent_regression_check.errors.messages
  end

  test 'AgentRegressionCheck destroy' do
    agent_regression_check = agent_regression_checks(:weather_test_forecast_tomorrow)
    agent_regression_check_id = agent_regression_check.id

    assert_equal 1, AgentRegressionCheck.where(id: agent_regression_check_id).count
    assert agent_regression_check.destroy
    assert_equal 0, AgentRegressionCheck.where(id: agent_regression_check_id).count
  end
end
