require 'test_helper'
require 'model_test_helper'

class AgentSearchTest < ActiveSupport::TestCase


  test "Search agent empty" do
    user = users(:admin)
    s = AgentSearch.new(user)
    assert_equal 5, s.options.size
    assert_equal user.id, s.options[:user_id]
    assert_equal 'popularity', s.options[:sort_by]
    assert_equal 'all', s.options[:filter_owner]
    assert_equal 'all', s.options[:filter_visibility]
    assert s.empty?
  end


  test "Search agent by name" do
    user = users(:admin)
    s = AgentSearch.new(user)
    assert_equal 2, Agent.search(s.options).count
    s = AgentSearch.new(user, query: '800')
    assert_equal 1, Agent.search(s.options).count
    expected = [
      'terminator'
    ]
    assert_equal expected, Agent.search(s.options).all.collect(&:agentname)
  end


  test "Search agent by agentname" do
    user = users(:admin)
    s = AgentSearch.new(user)
    assert_equal 2, Agent.search(s.options).count
    s = AgentSearch.new(user, query: 'inator')
    assert_equal 1, Agent.search(s.options).count
    expected = [
      'terminator'
    ]
    assert_equal expected, Agent.search(s.options).all.collect(&:agentname)
  end


  test 'Search agent is case insensitive' do
    user = users(:admin)
    s = AgentSearch.new(user)
    assert_equal 2, Agent.search(s.options).count
    s = AgentSearch.new(user, query: 'wEaTheR')
    assert_equal 1, Agent.search(s.options).count
    expected = [
      'weather'
    ]
    assert_equal expected, Agent.search(s.options).all.collect(&:agentname)
  end


  test 'Search agents description' do
    user = users(:admin)
    s = AgentSearch.new(user)
    assert_equal 2, Agent.search(s.options).count
    s = AgentSearch.new(user, query: 'description')
    assert_equal 1, Agent.search(s.options).count
    expected = [
      'weather'
    ]
    assert_equal expected, Agent.search(s.options).all.collect(&:agentname)
  end


  test 'Search agents by user name' do
    user = users(:admin)
    s = AgentSearch.new(user, query: 'admin')
    assert_equal 2, Agent.search(s.options).count
    expected = [
      'My awesome weather bot',
      'T-800',
    ]
    assert_equal expected, Agent.search(s.options).all.collect(&:name)

    s = AgentSearch.new(user, query: 'confirmed')
    assert_equal 0, Agent.search(s.options).count
    user = users(:admin)
    agent_public = agents(:weather_confirmed)
    agent_public.visibility = 'is_public'
    assert agent_public.save
    s = AgentSearch.new(user, query: 'confirmed')
    assert_equal 1, Agent.search(s.options).count
    expected = [
      'Weather bot'
    ]
    assert_equal expected, Agent.search(s.options).all.collect(&:name)
  end


  test "Search agent by name is trimmed" do
    user = users(:admin)
    s = AgentSearch.new(user)
    assert_equal 2, Agent.search(s.options).count
    s = AgentSearch.new(user, query: ' inator     ')
    assert_equal 1, Agent.search(s.options).count
    expected = [
      'terminator'
    ]
    assert_equal expected, Agent.search(s.options).all.collect(&:agentname)
  end


  test 'Search agents with membership or public' do
    user = users(:admin)
    agent_public = agents(:weather_confirmed)
    agent_public.visibility = 'is_public'
    assert agent_public.save
    s = AgentSearch.new(user)
    assert_equal 3, Agent.search(s.options).count
  end


  test 'Filter public agents' do
    user = users(:admin)
    s = AgentSearch.new(user, filter_visibility: 'public')
    assert_equal 1, Agent.search(s.options).count
    expected = [
      'terminator'
    ]
    assert_equal expected, Agent.search(s.options).all.collect(&:agentname)
  end


  test 'Filter private agents' do
    user = users(:admin)
    s = AgentSearch.new(user, filter_visibility: 'private')
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
    user = users(:admin)
    s = AgentSearch.new(user, filter_owner: 'owned')
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
    user = users(:admin)
    s = AgentSearch.new(user, filter_owner: 'all')
    assert_equal 3, Agent.search(s.options).count
    expected = [
      'My awesome weather bot',
      'T-800',
      'Weather bot'
    ]
    assert_equal expected, Agent.search(s.options).all.collect(&:name)
  end


  test 'Filter favorites agents' do
    agent_public = agents(:weather_confirmed)
    agent_public.visibility = Agent.visibilities[:is_public]
    assert agent_public.save
    admin = users(:admin)
    assert FavoriteAgent.create(user: admin, agent: agent_public)
    assert FavoriteAgent.create(user: admin, agent: agents(:weather))

    s = AgentSearch.new(admin, filter_owner: 'favorites')
    assert_equal 2, Agent.search(s.options).count
    expected = [
      'admin/weather',
      'confirmed/weather'
    ]
    assert_equal expected, Agent.search(s.options).all.collect(&:slug)
  end


  test 'Search agent and sort result' do
    user = users(:admin)
    s = AgentSearch.new(user)
    assert_equal 2, Agent.search(s.options).count
    s = AgentSearch.new(user, query: 'er', sort_by: 'updated_at')
    assert_equal 2, Agent.search(s.options).count
    expected = [
      'weather',
      'terminator'
    ]
    assert_equal expected, Agent.search(s.options).all.collect(&:agentname)
  end


  test 'Init agent search with user state' do
    user = users(:admin)
    user.ui_state = {
      agent_search: {
        filter_owner: 'owned',
        filter_visibility: 'private',
        sort_by: 'updated_at'
      }
    }
    assert user.save
    s = AgentSearch.new(user)
    assert_equal 'owned', s.options[:filter_owner]
    assert_equal 'private', s.options[:filter_visibility]
    assert_equal 'updated_at', s.options[:sort_by]
  end


  test 'Agent search criteria have precedence over user state' do
    user = users(:admin)
    user.ui_state = {
      agent_search: {
        filter_owner: 'owned',
        filter_visibility: 'private',
        sort_by: 'updated_at'
      }
    }
    assert user.save
    criteria = {
      'filter_owner' => 'all',
      'filter_visibility' => 'public',
      'sort_by' => 'agentname'
    }
    s = AgentSearch.new(user, criteria)
    assert_equal 'all', s.options[:filter_owner]
    assert_equal 'public', s.options[:filter_visibility]
    assert_equal 'agentname', s.options[:sort_by]
  end


  test 'Save search criteria in user state' do
    user = users(:admin)
    assert_equal user.ui_state, {}
    criteria = {
      'filter_owner' => 'owned',
      'filter_visibility' => 'private',
      'query' => 'weath',
      'sort_by' => 'updated_at'
    }
    s = AgentSearch.new(user, criteria)
    assert_equal 1, Agent.search(s.options).count
    assert s.save
    force_reset_model_cache(user)
    assert_equal criteria.with_indifferent_access, user.ui_state['agent_search']
  end


  test 'Empty agent search' do
    user = users(:admin)
    assert AgentSearch.new(user).empty?

    user.ui_state = {
      agent_search: {
        filter_owner: 'owned',
        filter_visibility: 'private',
        sort_by: 'updated_at'
      }
    }
    assert user.save
    assert !AgentSearch.new(user).empty?

    user.ui_state = {}
    assert user.save
    criteria = {
      'filter_owner' => 'owned',
      'filter_visibility' => 'private',
      'query' => 'weather',
      'sort_by' => 'updated_at'
    }
    assert !AgentSearch.new(user, criteria).empty?
  end

  test 'Sort agents by popularity' do
    #                  C (public)
    #                  |
    #                  Z (member)
    #                /  \
    # (owner) Weather   Terminator (owner)
    user = users(:admin)
    weather = agents(:weather)
    terminator = agents(:terminator)

    agent_z = create_agent('Agent Z', :confirmed)
    membership = MembershipsCreator.new(agent_z, user.username, 'show')
    assert membership.create

    assert AgentArc.create(agent: weather, depends_on: agent_z)
    assert AgentArc.create(agent: terminator, depends_on: agent_z)

    agent_c = create_agent('Agent C')
    agent_c.visibility = 'is_public'
    assert agent_c.save
    assert AgentArc.create(agent: agent_z, depends_on: agent_c)

    user = users(:admin)
    s = AgentSearch.new(user, sort_by: 'popularity')
    assert_equal 4, Agent.search(s.options).count
    expected = [
      'Agent Z',
      'Agent C',
      'My awesome weather bot',
      'T-800'
    ]
    assert_equal expected, Agent.search(s.options).all.collect(&:name)
  end
end
