require 'test_helper'

class InterpretRequestReporterTest < ActiveSupport::TestCase

  test 'API Requests basic count' do
    user = users(:admin)
    agent = agents(:weather)

    create_fixture(agent, '2019-01-01T12:00:00.000+02:00', '200')
    create_fixture(agent, '2019-01-02T12:00:00.000+02:00', '200')
    create_fixture(agent, '2019-01-03T12:00:00.000+02:00', '429')

    from = DateTime.parse('2019-01-01T00:00:00.000+02:00')
    to   = DateTime.parse('2019-01-04T00:00:00.000+02:00')

    assert_equal 2, InterpretRequestReporter.new
      .with_owner(user.id)
      .between(from, to)
      .under_quota
      .count

    assert_equal 1, InterpretRequestReporter.new
      .with_owner(user.id)
      .between(from, to)
      .over_quota
      .count


    from = DateTime.parse('2019-01-01T00:00:00.000+02:00')
    to   = DateTime.parse('2019-01-02T00:00:00.000+02:00')

    assert_equal 1, InterpretRequestReporter.new
      .with_owner(user.id)
      .between(from, to)
      .under_quota
      .count

    assert_equal 0, InterpretRequestReporter.new
      .with_owner(user.id)
      .between(from, to)
      .over_quota
      .count
  end


  test 'API Requests count per hours' do
    user = users(:admin)
    agent = agents(:weather)

    create_fixture(agent, '2019-01-01T00:00:00.000+02:00', '200')
    create_fixture(agent, '2019-01-01T00:00:01.000+02:00', '200')
    create_fixture(agent, '2019-01-01T00:00:03.000+02:00', '429')
    create_fixture(agent, '2019-01-01T01:00:00.100+02:00', '200')
    create_fixture(agent, '2019-01-01T01:01:00.100+02:00', '200')
    create_fixture(agent, '2019-01-01T01:02:00.100+02:00', '200')

    from = DateTime.parse('2019-01-01T00:00:00.000+02:00')
    to   = DateTime.parse('2019-01-01T02:00:00.000+02:00')

    expected = [
      {
          "key_as_string" => "2019-01-01T00:00:00.000+02:00",
                    "key" => 1546293600000,
              "doc_count" => 2
      },
      {
          "key_as_string" => "2019-01-01T01:00:00.000+02:00",
                    "key" => 1546297200000,
              "doc_count" => 3
      },
      {
          "key_as_string" => "2019-01-01T02:00:00.000+02:00",
                    "key" => 1546300800000,
              "doc_count" => 0
      }
    ]
    assert_equal expected, InterpretRequestReporter.new
      .with_owner(user.id)
      .between(from, to)
      .under_quota
      .count_per_hours

    expected = [
      {
          "key_as_string" => "2019-01-01T00:00:00.000+02:00",
                    "key" => 1546293600000,
              "doc_count" => 1
      },
      {
          "key_as_string" => "2019-01-01T01:00:00.000+02:00",
                    "key" => 1546297200000,
              "doc_count" => 0
      },
      {
          "key_as_string" => "2019-01-01T02:00:00.000+02:00",
                    "key" => 1546300800000,
              "doc_count" => 0
      }
    ]
    assert_equal expected, InterpretRequestReporter.new
      .with_owner(user.id)
      .between(from, to)
      .over_quota
      .count_per_hours
  end


  test 'API Requests count per agents per days' do
    user = users(:admin)
    weather = agents(:weather)
    terminator = agents(:terminator)

    create_fixture(weather, '2019-01-01T00:00:00.000+02:00', '200')
    create_fixture(weather, '2019-01-01T00:00:01.000+02:00', '200')
    create_fixture(weather, '2019-01-02T00:00:00.000+02:00', '429')
    create_fixture(terminator, '2019-01-03T01:00:00.000+02:00', '200')

    from = DateTime.parse('2019-01-01T00:00:00.000+02:00')
    to   = DateTime.parse('2019-01-03T23:59:59.999+02:00')

    under_quota = InterpretRequestReporter.new
      .with_owner(user.id)
      .between(from, to)
      .under_quota
      .count_per_agent_and_per_day

    assert_equal 2, under_quota.size
    assert_equal weather.slug, under_quota.first["key"]
    assert_equal 2, under_quota.first["doc_count"]
    assert_equal 3, under_quota.first["requests"]["buckets"].size
    assert_equal [2, 0, 0], under_quota.first["requests"]["buckets"].collect{|d| d["doc_count"]}

    assert_equal terminator.slug, under_quota.last["key"]
    assert_equal 1, under_quota.last["doc_count"]
    assert_equal 3, under_quota.last["requests"]["buckets"].size
    assert_equal [0, 0, 1], under_quota.last["requests"]["buckets"].collect {|d| d["doc_count"]}

    over_quota = InterpretRequestReporter.new
      .with_owner(user.id)
      .between(from, to)
      .over_quota
      .count_per_agent_and_per_day

    assert_equal 1, over_quota.size
    assert_equal weather.slug, over_quota.first["key"]
    assert_equal 1, over_quota.first["doc_count"]
    assert_equal 3, over_quota.first["requests"]["buckets"].size
    assert_equal [0, 1, 0], over_quota.first["requests"]["buckets"].collect{|d| d["doc_count"]}

    all_api = InterpretRequestReporter.new
      .with_owner(user.id)
      .between(from, to)
      .from_api
      .count_per_agent_and_per_day

    assert_equal 2, all_api.size
    assert_equal weather.slug, all_api.first["key"]
    assert_equal 3, all_api.first["doc_count"]
    assert_equal [2, 1, 0], all_api.first["requests"]["buckets"].collect{|d| d["doc_count"]}

    assert_equal terminator.slug, all_api.last["key"]
    assert_equal 1, all_api.last["doc_count"]
    assert_equal 3, all_api.last["requests"]["buckets"].size
    assert_equal [0, 0, 1], all_api.last["requests"]["buckets"].collect {|d| d["doc_count"]}
  end


  def create_fixture(agent, date, code)
    log = InterpretRequestLog.new(
      timestamp: date,
      sentence: "Hello",
      agents: [agent],
    ).with_response(code, {})
    assert log.save
  end

end
