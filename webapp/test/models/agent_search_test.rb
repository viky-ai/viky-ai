require 'test_helper'

class AgentSearchTest < ActiveSupport::TestCase


  test "Search agent empty" do
    user_id = users(:admin).id
    s = AgentSearch.new(user_id)
    assert_equal 4, s.options.size
    assert_equal user_id, s.options[:user_id]
    assert_equal 'name', s.options[:sort_by]
    assert_equal 'all', s.options[:filter_owner]
    assert_equal 'all', s.options[:filter_visibility]
    assert s.empty?
  end


  test "Search agent by name" do
    user_id = users(:admin).id
    s = AgentSearch.new(user_id)
    assert_equal 2, Agent.search(s.options).count
    s = AgentSearch.new(user_id, query: '800')
    assert_equal 1, Agent.search(s.options).count
    expected = [
      'terminator'
    ]
    assert_equal expected, Agent.search(s.options).all.collect(&:agentname)
  end


  test "Search agent by agentname" do
    user_id = users(:admin).id
    s = AgentSearch.new(user_id)
    assert_equal 2, Agent.search(s.options).count
    s = AgentSearch.new(user_id, query: 'inator')
    assert_equal 1, Agent.search(s.options).count
    expected = [
      'terminator'
    ]
    assert_equal expected, Agent.search(s.options).all.collect(&:agentname)
  end


  test 'Search agent is case insensitive' do
    user_id = users(:admin).id
    s = AgentSearch.new(user_id)
    assert_equal 2, Agent.search(s.options).count
    s = AgentSearch.new(user_id, query: 'wEaTheR')
    assert_equal 1, Agent.search(s.options).count
    expected = [
      'weather'
    ]
    assert_equal expected, Agent.search(s.options).all.collect(&:agentname)
  end


  test 'Search agents description' do
    user_id = users(:admin).id
    s = AgentSearch.new(user_id)
    assert_equal 2, Agent.search(s.options).count
    s = AgentSearch.new(user_id, query: 'description')
    assert_equal 1, Agent.search(s.options).count
    expected = [
      'weather'
    ]
    assert_equal expected, Agent.search(s.options).all.collect(&:agentname)
  end


  test "Search agent by name is trimmed" do
    user_id = users(:admin).id
    s = AgentSearch.new(user_id)
    assert_equal 2, Agent.search(s.options).count
    s = AgentSearch.new(user_id, query: ' inator     ')
    assert_equal 1, Agent.search(s.options).count
    expected = [
      'terminator'
    ]
    assert_equal expected, Agent.search(s.options).all.collect(&:agentname)
  end


  test 'Search agents with membership or public' do
    user_id = users(:admin).id
    agent_public = agents(:weather_confirmed)
    agent_public.visibility = 'is_public'
    assert agent_public.save
    s = AgentSearch.new(user_id)
    assert_equal 3, Agent.search(s.options).count
  end


  test 'Filter public agents' do
    user_id = users(:admin).id
    s = AgentSearch.new(user_id, filter_visibility: 'public')
    assert_equal 1, Agent.search(s.options).count
    expected = [
      'terminator'
    ]
    assert_equal expected, Agent.search(s.options).all.collect(&:agentname)
  end


  test 'Filter private agents' do
    user_id = users(:admin).id
    s = AgentSearch.new(user_id, filter_visibility: 'private')
    assert_equal 1, Agent.search(s.options).count
    expected = [
      'weather'
    ]
    assert_equal expected, Agent.search(s.options).all.collect(&:agentname)
  end


  test 'Filter user owned agents' do
    agent_public = agents(:weather_confirmed)
    agent_public.visibility = Agent.visibilities[:is_public]
    assert agent_public.save
    user_id = users(:admin).id
    s = AgentSearch.new(user_id, filter_owner: 'owned')
    assert_equal 2, Agent.search(s.options).count
    expected = [
      'My awesome weather bot',
      'T-800'
    ]
    assert_equal expected, Agent.search(s.options).all.collect(&:name)
  end


  test 'Filter all agents' do
    agent_public = agents(:weather_confirmed)
    agent_public.visibility = Agent.visibilities[:is_public]
    assert agent_public.save
    user_id = users(:admin).id
    s = AgentSearch.new(user_id, filter_owner: 'all')
    assert_equal 3, Agent.search(s.options).count
    expected = [
      'My awesome weather bot',
      'T-800',
      'Weather bot'
    ]
    assert_equal expected, Agent.search(s.options).all.collect(&:name)
  end


  test 'Search agent and sort result' do
    user_id = users(:admin).id
    s = AgentSearch.new(user_id)
    assert_equal 2, Agent.search(s.options).count
    s = AgentSearch.new(user_id, query: 'er', sort_by: 'updated_at')
    assert_equal 2, Agent.search(s.options).count
    expected = [
      'weather',
      'terminator'
    ]
    assert_equal expected, Agent.search(s.options).all.collect(&:agentname)
  end
end
