require 'test_helper'

class StatisticsIndexTest < ActiveSupport::TestCase

  test 'Build from name' do
    index_active = StatisticsIndex.from_name('stats-interpret_request_log-active-4-29e0682e')
    assert_equal 'stats-interpret_request_log-active-4-29e0682e', index_active.name
    assert_equal 'active', index_active.state
    assert_equal 4, index_active.version
    assert index_active.active?

    index_inactive = StatisticsIndex.from_name('stats-interpret_request_log-inactive-2-30a1528f')
    assert_equal 'stats-interpret_request_log-inactive-2-30a1528f', index_inactive.name
    assert_equal 'inactive', index_inactive.state
    assert_equal 2, index_inactive.version
    assert_not index_inactive.active?
  end

  test 'Detect index need reindexing' do
    template = Struct.new(:version).new(2)
    index_active = StatisticsIndex.from_name('stats-interpret_request_log-active-1-29e0682e')
    assert index_active.need_reindexing?(template)

    index_inactive = StatisticsIndex.from_name('stats-interpret_request_log-inactive-1-30a1528f')
    assert index_inactive.need_reindexing?(template)
  end
end
