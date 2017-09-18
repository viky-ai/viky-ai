require 'test_helper'

class TestNlsTimeout < Minitest::Test

  include Common

  def setup
    Nls.start
  end

  def after_run
    Nls.stop
  end

#  def test_timeout
#
#    data = {
#          wait: 5000
#        }
#    actual = nls_query(data)
#
#    expected = {
#      "error" => ["Request takes too long"]
#    }
#
#    assert_equal expected, actual
#  end


end
