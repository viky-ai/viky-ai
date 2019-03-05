require 'test_helper'

class StatisticsIndexTest < ActiveSupport::TestCase

  test 'Build from name' do
    index_active = StatisticsIndex.from_name('stats-interpret_request_log-active-4-29e0682e')
    assert_equal 'stats-interpret_request_log-active-4-29e0682e', index_active.name
    assert_equal 'active', index_active.state
    assert_equal 4, index_active.version
    assert index_active.active?
    assert_not index_active.snapshot?

    index_inactive = StatisticsIndex.from_name('stats-interpret_request_log-inactive-2-30a1528f')
    assert_equal 'stats-interpret_request_log-inactive-2-30a1528f', index_inactive.name
    assert_equal 'inactive', index_inactive.state
    assert_equal 2, index_inactive.version
    assert_not index_inactive.active?
    assert_not index_inactive.snapshot?
  end


  test 'Build from name with snapshot' do
    index_active = StatisticsIndex.from_name('stats-interpret_request_log-active-4-5fadc4d3-29e0682e')
    assert_equal 'stats-interpret_request_log-active-4-5fadc4d3-29e0682e', index_active.name
    assert_equal 'active', index_active.state
    assert_equal 4, index_active.version
    assert index_active.active?
    assert index_active.snapshot?
    assert_equal '5fadc4d3', index_active.snapshot_id

    index_inactive = StatisticsIndex.from_name('stats-interpret_request_log-inactive-2-5fadc4d3-30a1528f')
    assert_equal 'stats-interpret_request_log-inactive-2-5fadc4d3-30a1528f', index_inactive.name
    assert_equal 'inactive', index_inactive.state
    assert_equal 2, index_inactive.version
    assert_not index_inactive.active?
    assert index_inactive.snapshot?
    assert_equal '5fadc4d3', index_inactive.snapshot_id
  end


  test 'Detect index need reindexing' do
    template = Struct.new(:version).new(2)
    index_active = StatisticsIndex.from_name('stats-interpret_request_log-active-1-29e0682e')
    assert index_active.need_reindexing?(template)

    index_inactive = StatisticsIndex.from_name('stats-interpret_request_log-inactive-1-5fadc4d3-30a1528f')
    assert index_inactive.need_reindexing?(template)
  end


  test 'Set a snapshot id' do
    index_active = StatisticsIndex.from_name('stats-interpret_request_log-active-1-29e0682e')
    assert_equal 'stats-interpret_request_log-active-1-29e0682e', index_active.name
    assert_not index_active.snapshot?

    index_active.snapshot_id = 'b10cf23a'
    assert_equal 'stats-interpret_request_log-active-1-b10cf23a-29e0682e', index_active.name
    assert index_active.snapshot?
  end
end
