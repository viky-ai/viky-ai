require 'test_helper'

class InterpretRequestLogTest < ActiveSupport::TestCase

  setup do
    IndexManager.reset_statistics
  end

  test 'Save a basic log' do
    weather_agent = agents(:weather)
    log = InterpretRequestLog.new(
      timestamp: '2018-07-04T14:25:14+02:00',
      sentence: "What 's the weather like ?",
      language: 'en',
      agent: weather_agent
    )
    assert log.save
    assert_equal 1, InterpretRequestLog.count
  end

  test 'Count how much interpret request in a specific time frame' do
    weather_agent = agents(:weather)
    log = InterpretRequestLog.new(
      timestamp: '2018-07-04T08:00:00+02:00',
      sentence: 'What the weather like today ?',
      language: 'en',
      agent: weather_agent
    )
    assert log.save
    log = InterpretRequestLog.new(
      timestamp: '2018-07-04T12:00:00+02:00',
      sentence: 'What the weather like tomorrow ?',
      language: 'en',
      agent: weather_agent
    )
    assert log.save
    log = InterpretRequestLog.new(
      timestamp: '2018-07-04T17:00:00+02:00',
      sentence: 'What the weather like next Sunday ?',
      language: 'en',
      agent: weather_agent
    )
    assert log.save
    assert_equal 2, InterpretRequestLog.count(
      query: {
        range: {
          timestamp: {
            gte: '2018-07-04T10:00:00+02:00',
            lte: '2018-07-04T18:00:00+02:00'
          }
        }
      }
    )
  end
end
