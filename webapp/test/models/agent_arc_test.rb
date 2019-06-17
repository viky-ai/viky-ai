require 'test_helper'
require 'model_test_helper'

class AgentArcTest < ActiveSupport::TestCase

  test "basic agents association" do
    agent_a = create_agent("Agent A")
    agent_b = create_agent("Agent B")

    assert AgentArc.create(source: agent_a, target: agent_b)

    #   A
    #  /
    # B
    force_reset_model_cache([agent_a, agent_b])
    assert_equal [], agent_a.predecessors.collect(&:name)
    assert_equal ["Agent B"], agent_a.successors.collect(&:name)

    assert_equal ["Agent A"], agent_b.predecessors.collect(&:name)
    assert_equal [], agent_b.successors.collect(&:name)


    agent_c = create_agent("Agent C")
    assert AgentArc.create(source: agent_a, target: agent_c)
    #   A
    #  / \
    # B   C
    force_reset_model_cache([agent_a, agent_b, agent_c])
    assert_equal [], agent_a.predecessors.collect(&:name)
    assert_equal ["Agent B", "Agent C"].sort, agent_a.successors.collect(&:name).sort

    assert_equal ["Agent A"], agent_b.predecessors.collect(&:name)
    assert_equal [], agent_b.successors.collect(&:name)

    assert_equal ["Agent A"], agent_c.predecessors.collect(&:name)
    assert_equal [], agent_c.successors.collect(&:name)


    agent_d = create_agent("Agent D")
    assert AgentArc.create(source: agent_b, target: agent_d)
    #   A
    #  / \
    # B   C
    # |
    # D
    force_reset_model_cache([agent_a, agent_b, agent_c, agent_d])
    assert_equal [], agent_a.predecessors.collect(&:name)
    assert_equal ["Agent B", "Agent C"].sort, agent_a.successors.collect(&:name).sort

    assert_equal ["Agent A"], agent_b.predecessors.collect(&:name)
    assert_equal ["Agent D"], agent_b.successors.collect(&:name)

    assert_equal ["Agent A"], agent_c.predecessors.collect(&:name)
    assert_equal [], agent_c.successors.collect(&:name)

    assert_equal ["Agent B"], agent_d.predecessors.collect(&:name)
    assert_equal [], agent_d.successors.collect(&:name)


    assert AgentArc.create(source: agent_c, target: agent_d)
    #   A
    #  / \
    # B   C
    # | /
    # D
    force_reset_model_cache([agent_a, agent_b, agent_c, agent_d])
    assert_equal [], agent_a.predecessors.collect(&:name)
    assert_equal ["Agent B", "Agent C"].sort, agent_a.successors.collect(&:name).sort

    assert_equal ["Agent A"], agent_b.predecessors.collect(&:name)
    assert_equal ["Agent D"], agent_b.successors.collect(&:name)

    assert_equal ["Agent A"], agent_c.predecessors.collect(&:name)
    assert_equal ["Agent D"], agent_c.successors.collect(&:name)

    assert_equal ["Agent B", "Agent C"].sort, agent_d.predecessors.collect(&:name).sort
    assert_equal [], agent_d.successors.collect(&:name)


    assert_equal 4, AgentArc.count
    assert Agent.find_by_name("Agent B").destroy
    #   A
    #   |
    #   C
    #   |
    #   D
    force_reset_model_cache([agent_a, agent_c, agent_d])
    assert_equal [], agent_a.predecessors.collect(&:name)
    assert_equal ["Agent C"], agent_a.successors.collect(&:name)

    assert_equal ["Agent A"], agent_c.predecessors.collect(&:name)
    assert_equal ["Agent D"], agent_c.successors.collect(&:name)

    assert_equal ["Agent C"], agent_d.predecessors.collect(&:name)
    assert_equal [], agent_d.successors.collect(&:name)

    assert_equal 2, AgentArc.count
  end


  test 'Ensure no cycle is present' do
    #   A
    #  / \
    # B   C
    agent_a = create_agent('Agent A')
    agent_b = create_agent('Agent B')
    agent_c = create_agent('Agent C')

    AgentArc.create(source: agent_a, target: agent_b)
    AgentArc.create(source: agent_c, target: agent_a)

    force_reset_model_cache([agent_a, agent_b, agent_c])

    arc = AgentArc.new(source: agent_b, target: agent_c)

    assert arc.invalid?
    expected = ['Cycle detected, agent addition would cause a cycle in dependency graph']
    assert_equal expected, arc.errors.full_messages
  end


  test 'AgentArc must be unique' do
    #   A
    #  /
    # B
    agent_a = create_agent('Agent A')
    agent_b = create_agent('Agent B')

    AgentArc.create(source: agent_a, target: agent_b)
    arc = AgentArc.new(source: agent_a, target: agent_b)

    assert arc.invalid?
    expected = ['Target has already been taken']
    assert_equal expected, arc.errors.full_messages
  end


  test 'AgentArc src and target must be present' do
    arc = AgentArc.new
    assert arc.invalid?
    expected = ['Source must exist',
                'Source can\'t be blank',
                'Target must exist',
                'Target can\'t be blank'].sort
    assert_equal expected, arc.errors.full_messages.sort
  end


  test 'Agent available successors' do
    agent_a = create_agent('Agent A')
    agent_b = create_agent('Agent B')
    agent_b.visibility = 'is_public'
    agent_b.memberships << Membership.new(user_id: users(:edit_on_agent_weather).id, rights: "edit")
    assert agent_b.save

    current_user = users(:admin)
    search = AgentSelectSearch.new(current_user)
    successors = agent_a.available_successors(search.options).order(name: :asc)
    expected = [
      'Agent B',
      'My awesome weather bot',
      'T-800'
    ]
    assert_equal expected, successors.order(name: :asc).collect(&:name)
  end


  test 'Agent search favorite available successors' do
    current_user = users(:admin)
    agent_a = create_agent('Agent A')

    agent_b = create_agent('Agent B')
    assert FavoriteAgent.create(user: current_user, agent: agent_b)

    agent_public = agents(:weather_confirmed)
    agent_public.visibility = 'is_public'
    assert agent_public.save
    assert FavoriteAgent.create(user: current_user, agent: agent_public)

    agent_c = create_agent('Agent C')
    agent_c.visibility = 'is_public'
    assert agent_c.save
    assert FavoriteAgent.create(user: users(:edit_on_agent_weather), agent: agent_c)

    search = AgentSelectSearch.new(current_user, filter_owner: 'favorites')
    successors = agent_a.available_successors(search.options).order(name: :asc)
    expected = [
      'admin/agent-b',
      'confirmed/weather'
    ]
    assert_equal expected, successors.collect(&:slug)
  end


  test 'Agent search with query available successors' do
    agent_a = create_agent('Agent A')
    create_agent('Agent 212')
    current_user = users(:admin)
    search = AgentSelectSearch.new(current_user, query: 'agent')
    successors = agent_a.available_successors(search.options)
    expected = [
      'admin/agent-212'
    ]
    assert_equal expected, successors.collect(&:slug)
  end


  test 'Agent available successors with public agent' do
    agent_a = create_agent('Agent A')
    create_agent('Agent B')

    agent_public = agents(:weather_confirmed)
    agent_public.visibility = 'is_public'
    assert agent_public.save

    current_user = users(:admin)
    search = AgentSelectSearch.new(current_user)
    successors = agent_a.available_successors(search.options).order(name: :asc)
    expected = [
      'Agent B',
      'My awesome weather bot',
      'T-800',
      'Weather bot'
    ]
    assert_equal expected, successors.order(name: :asc).collect(&:name)
  end


  test 'Empty successor search' do
    user = users(:admin)
    assert AgentSelectSearch.new(user).empty?

    criteria = {
      'filter_owner' => 'owned',
      'query' => 'weather'
    }
    assert !AgentSelectSearch.new(user, criteria).empty?
  end


  test 'Sync with NLP when a dependency is added' do
    Nlp::Package.any_instance.expects(:push).at_least(3)
    agent_a = create_agent('Agent A')
    agent_b = create_agent('Agent B')
    force_reset_model_cache([agent_a, agent_b])

    assert AgentArc.create(source: agent_a, target: agent_b)
  end

  test 'Alias attributes for more readable code' do
    agent_a = create_agent("Agent A")
    agent_b = create_agent("Agent B")

    assert AgentArc.create agent: agent_a, depends_on: agent_b
  end
end
