require 'test_helper'

class InterpretRequestLogTest < ActiveSupport::TestCase

  setup do
    IndexManager.reset_statistics
  end

  test 'Save a basic log' do
    log = InterpretRequestLog.new('2018-07-04T14:25:14+02:00', "What 's the weather like ?", 'en')
    assert log.save
    sleep 1
    assert_equal 1, InterpretRequestLog.count
  end

  test 'Count how much interpret request in a specific time frame' do
    log = InterpretRequestLog.new('2018-07-04T08:00:00+02:00', 'What the weather like today ?', 'en')
    assert log.save
    log = InterpretRequestLog.new('2018-07-04T12:00:00+02:00', 'What the weather like tomorrow ?', 'en')
    assert log.save
    log = InterpretRequestLog.new('2018-07-04T17:00:00+02:00', 'What the weather like next Sunday ?', 'en')
    assert log.save
    sleep 1
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
