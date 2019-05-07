require 'test_helper'
require 'model_test_helper'

class AgentRegressionCheckTest < ActiveSupport::TestCase

  setup do
    create_agent_regression_check_fixtures
  end

  test 'Basic AgentRegressionCheck creation & agent association' do
    assert_equal 3, agents(:weather).agent_regression_checks.count

    expected_nlp = {
      'root_type' => 'intent',
      'package' => intents(:weather_forecast).agent.id,
      'id' => intents(:weather_forecast).id,
      'solution' => interpretations(:weather_forecast_tomorrow).solution.to_json.to_s
    }.with_indifferent_access
    agent_regression_check = AgentRegressionCheck.new(
      sentence: 'What the weather like ?',
      language: 'fr',
      spellchecking: 'low',
      now: '2019-01-17T09:00:00+01:00',
      expected: expected_nlp,
      got: expected_nlp,
      state: 'success',
      position: 2,
      agent: agents(:weather)
    )
    assert agent_regression_check.save
    assert_equal 4, agents(:weather).agent_regression_checks.count

    assert_equal 'What the weather like ?', agent_regression_check.sentence
    assert_equal 'fr', agent_regression_check.language
    assert_equal 'low', agent_regression_check.spellchecking
    assert_equal DateTime.parse('2019-01-17T09:00:00+01:00'), agent_regression_check.now
    assert_equal expected_nlp, agent_regression_check.expected
    assert_equal expected_nlp, agent_regression_check.got
    assert_equal 'success', agent_regression_check.state
    assert agent_regression_check.success?
    assert_not agent_regression_check.unknown?
    assert_not agent_regression_check.failure?
    assert_equal 2, agent_regression_check.position
  end


  test 'Agent, sentence and expected are mandatory' do
    agent_regression_check = AgentRegressionCheck.new(state: 'failure')
    assert_not agent_regression_check.save

    expected = {
      agent: ['must exist'],
      sentence: ["can't be blank"],
      expected: ["can't be blank"]
    }
    assert_equal expected, agent_regression_check.errors.messages
  end


  test 'Sentence cleanup before save' do
    agent_regression_check = AgentRegressionCheck.new(
      agent: agents(:weather),
      sentence: ' Ecrire des tests tu dois ',
      expected: '{ "citation": true, "author": "Olivier D."}'
    )
    assert agent_regression_check.save
    assert_equal "Ecrire des tests tu dois", agent_regression_check.sentence
  end


  test 'Sentence maximum length' do
    agent_regression_check = AgentRegressionCheck.new(
      agent: agents(:weather),
      sentence: 'a' * 201,
      expected: '{}'
    )
    assert_not agent_regression_check.save

    expected = {
      sentence: ['is too long (maximum is 200 characters)']
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
    agent_regression_check = @regression_weather_question
    agent_regression_check_id = agent_regression_check.id

    assert_equal 1, AgentRegressionCheck.where(id: agent_regression_check_id).count
    assert agent_regression_check.destroy
    assert_equal 0, AgentRegressionCheck.where(id: agent_regression_check_id).count
  end


  test 'Running test should not be deleted' do
    @regression_weather_forecast.state = 'running'
    assert @regression_weather_forecast.save

    assert !@regression_weather_forecast.destroy
    expected_error = { base: ["Regression test cannot be deleted in running state."] }
    assert_equal expected_error, @regression_weather_forecast.errors.messages
  end


  test 'Run a successful agent test' do
    Nlp::Interpret.any_instance.stubs('proceed').returns(
      status: 200,
      body: {
        'interpretations' => [{
          'id' => intents(:weather_forecast).id,
          'slug' => intents(:weather_forecast).slug,
          'package' => intents(:weather_forecast).agent.id,
          'score' => '1.0',
          'solution' => interpretations(:weather_forecast_tomorrow).solution
        }, {
          'id' => intents(:weather_question).id,
          'slug' => intents(:weather_question).slug,
          'package' => intents(:weather_question).agent.id,
          'score' => '0.3',
          'solution' => { 'question' => 'what' }
        }]
      }
    )

    agent_regression_check = @regression_weather_forecast
    assert agent_regression_check.run

    assert agent_regression_check.success?
    assert_not agent_regression_check.failure?
    assert_not agent_regression_check.unknown?
    assert_not agent_regression_check.running?
    expected = {
      'root_type' => 'intent',
      'package' => intents(:weather_forecast).agent.id,
      'id' => intents(:weather_forecast).id,
      'solution' => interpretations(:weather_forecast_tomorrow).solution.to_json.to_s
    }
    assert_equal expected, agent_regression_check.expected
    assert_equal agent_regression_check.got, agent_regression_check.expected
  end


  test 'Run a failed agent test' do
    Nlp::Interpret.any_instance.stubs('proceed').returns(
      status: 200,
      body: {
        'interpretations' => [{
          'id' => intents(:weather_forecast).id,
          'slug' => intents(:weather_forecast).slug,
          'package' => intents(:weather_forecast).agent.id,
          'score' => '0.3',
          'solution' => interpretations(:weather_forecast_tomorrow).solution
        }]
      }
    )
    agent_regression_check = @regression_weather_question
    assert agent_regression_check.run

    assert_not agent_regression_check.success?
    assert agent_regression_check.failure?
    assert_not agent_regression_check.running?
    assert_not agent_regression_check.unknown?
    assert_not_equal agent_regression_check.got, agent_regression_check.expected
  end


  test 'Run a succes agent test but empty nothing match' do
    Nlp::Interpret.any_instance.stubs('proceed').returns(
      status: 200,
      body: {
        'interpretations' => []
      }
    )
    agent_regression_check = @regression_weather_question
    assert agent_regression_check.run

    assert_not agent_regression_check.success?
    assert agent_regression_check.failure?
    assert_not agent_regression_check.running?
    assert_not agent_regression_check.unknown?
    assert_not agent_regression_check.error?
    assert_not_equal agent_regression_check.got, agent_regression_check.expected
  end


  test 'Run an agent test but NLP is unreachable' do
    Nlp::Interpret.any_instance.stubs('proceed').returns(
      status: 404,
      body: {}
    )
    agent_regression_check = @regression_weather_forecast
    assert agent_regression_check.run

    assert_not agent_regression_check.success?
    assert_not agent_regression_check.failure?
    assert_not agent_regression_check.running?
    assert agent_regression_check.unknown?
    assert_empty agent_regression_check.got
  end


  test 'Run an erroneous agent test' do
    Nlp::Interpret.any_instance.stubs('proceed').returns(
      status: 422,
      body: {}
    )
    agent_regression_check = @regression_weather_forecast
    assert agent_regression_check.run

    assert_not agent_regression_check.success?
    assert_not agent_regression_check.failure?
    assert_not agent_regression_check.unknown?
    assert_not agent_regression_check.running?
    assert agent_regression_check.error?
    assert_empty agent_regression_check.got
  end

test 'Run a successful agent test for entities list' do
      Nlp::Interpret.any_instance.stubs('proceed').returns(
        status: 200,
        body: {
          'interpretations' => [{
            'id' => entities_lists(:weather_conditions).id,
            'slug' => entities_lists(:weather_conditions).slug,
            'package' => entities_lists(:weather_conditions).agent.id,
            'score' => '1.0',
            'solution' => entities(:weather_sunny).solution
          }]
        }
      )

      agent_regression_check = @regression_weather_condition
      assert agent_regression_check.run

      assert agent_regression_check.success?
      assert_not agent_regression_check.failure?
      assert_not agent_regression_check.unknown?
      assert_not agent_regression_check.running?
      assert_equal agent_regression_check.got, agent_regression_check.expected
    end

end
