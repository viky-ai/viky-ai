require 'test_helper'

class InterpretRequestLogTest < ActiveSupport::TestCase

  setup do
    IndexManager.reset_indices
  end

  test 'Save a basic interpretation log' do
    weather_agent = agents(:weather)
    intent_weather = intents(:weather_question)
    log = InterpretRequestLog.new(
      timestamp: '2018-07-04T14:25:14.053+02:00',
      sentence: "What 's the weather like ?",
      language: 'en',
      agent: weather_agent,
    ).with_response('200', {
      'interpretations' => [{
        'id' => intent_weather.id,
        'slug' => intent_weather.slug,
        'name' => intent_weather.intentname,
        'score' => '0.83',
        'solution' => 'forecast.today'
      }]
    })
    assert log.save
    assert_equal 1, InterpretRequestLog.count
  end

  test 'Find an interpretation log by id' do
    weather_agent = agents(:weather)
    intent_weather = intents(:weather_question)
    log = InterpretRequestLog.new(
      timestamp: '2018-07-04T14:25:14+02:00',
      sentence: "What 's the weather like ?",
      language: 'en',
      now: '2018-07-10T14:10:36+02:00',
      agent: weather_agent,
    ).with_response('200', {
      'interpretations' => [{
        'id' => intent_weather.id,
        'slug' => intent_weather.slug,
        'name' => intent_weather.intentname,
        'score' => '0.83',
        'solution' => 'forecast.today'
      }]
    })
    assert log.save
    found = InterpretRequestLog.find(log.id)
    assert_equal log.timestamp, found.timestamp
    assert_equal log.sentence, found.sentence
    assert_equal log.language, found.language
    assert_equal log.now, found.now
    assert_equal log.status, found.status
    assert_equal log.body, found.body
  end

  test 'Count how much interpret request in a specific time frame' do
    weather_agent = agents(:weather)
    intent_weather_question = intents(:weather_question)
    intent_weather_forecast = intents(:weather_forecast)
    log = InterpretRequestLog.new(
      timestamp: '2018-07-04T08:00:00.200+02:00',
      sentence: 'What the weather like today ?',
      language: 'en',
      agent: weather_agent,
    ).with_response('200', {
      'interpretations' => [{
        'id' => intent_weather_question.id,
        'slug' => intent_weather_question.slug,
        'name' => intent_weather_question.intentname,
        'score' => '0.83',
        'solution' => 'forecast.today'
      }]
    })
    assert log.save
    log = InterpretRequestLog.new(
      timestamp: '2018-07-04T12:00:00.062+02:00',
      sentence: 'What the weather like tomorrow ?',
      language: 'en',
      agent: weather_agent,
    ).with_response('200', {
      'interpretations' => [{
        'id' => intent_weather_forecast.id,
        'slug' => intent_weather_forecast.slug,
        'name' => intent_weather_forecast.intentname,
        'score' => '0.83',
        'solution' => {
          'date' => 'tomorrow'
        }
      }]
    })
    assert log.save
    log = InterpretRequestLog.new(
      timestamp: '2018-07-04T16:00:00.000+02:00',
      sentence: 'What the weather like next Sunday ?',
      agent: weather_agent,
    ).with_response('422', {
      errors: [
        "lt 0: OgNlsEndpoints : request error on endpoint : \"POST NlsEndpointInterpret\"",
        "NlpInterpretRequestBuildPackage: unknown package 'd4f359ed-fbe6-450b-a4c8-f449f232a699'"
      ]
    })
    assert log.save
    log = InterpretRequestLog.new(
      timestamp: '2018-07-04T17:00:00.000+02:00',
      sentence: 'What the weather like next Sunday ?',
      language: 'en',
      agent: weather_agent,
    ).with_response('200', {})
    assert log.save
    assert_equal 2, InterpretRequestLog.count(
      query: {
        bool: {
          must_not: {
            term: { status: 422 }
          },
          must: {
            range: {
              timestamp: {
                gte: '2018-07-04T10:00:00+02:00',
                lte: '2018-07-04T18:00:00+02:00'
              }
            }
          }
        }
      }
    )
  end


  test 'Log an NLP response with an error message' do
    weather_agent = agents(:weather)
    log = InterpretRequestLog.new(
      timestamp: '2018-07-04T16:00:00.000+02:00',
      sentence: 'What the weather like next Sunday ?',
      agent: weather_agent,
    ).with_response('401', {
      errors: ['Access denied: wrong token.']
    })
    assert log.save
  end

  test 'Log an interpretation with a context info' do
    log = InterpretRequestLog.new(
        timestamp: '2018-11-21T16:00:00.000+02:00',
        sentence: 'What the weather like today ?',
        agent: agents(:weather),
        context: {'client_type' => 'console', 'user_id' => '078602e7-0578-49d4-96ee-d8ee2f9b1ecf'}
      ).with_response('200', {
        'interpretations' => [{
          'package': '132154f2-4545-4ae2-a802-3d39a2a5013f',
          'id' => 'a342f0ff-5d49-4a29-bc37-a9d754b99a7b',
          'slug' => 'viky/weather/interpretations/weather_question',
          'score' => '0.83',
          'solution' => {
            'date' => 'today'
          }
        }]
    })
    assert log.save
  end
end
