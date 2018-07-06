require 'test_helper'

class InterpretRequestLogTest < ActiveSupport::TestCase

  setup do
    IndexManager.reset_statistics
  end

  test 'Save a basic log' do
    log = InterpretRequestLog.new('2018-07-04T14:25:14+02:00', "What 's the weather like ?", 'en')
    assert log.save
  end
end
