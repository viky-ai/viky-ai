require 'test_helper'

class PlayAgentsSearchTest < ActiveSupport::TestCase

  test "Search agent query empty" do
    user = users(:admin)
    s = PlayAgentsSearch.new(user)
    assert_equal "", s.options[:query]
    assert_equal user.id, s.options[:user_id]
  end


  test "Search agent by name" do
    user = users(:admin)
    s = PlayAgentsSearch.new(user)
    assert_equal 2, Agent.search(s.options).count

    s = PlayAgentsSearch.new(user, query: '800')
    assert_equal 1, Agent.search(s.options).count

    expected = ['terminator']
    assert_equal expected, Agent.search(s.options).collect(&:agentname)
  end


  test "Search selected params" do
    user = users(:admin)

    s = PlayAgentsSearch.new(user, selected_ids: [agents(:terminator).id])
    expected = ['weather', 'terminator']
    assert_equal expected, Agent.search(s.options).order(name: :asc).collect(&:agentname)

    s = PlayAgentsSearch.new(user, selected_ids: [agents(:terminator).id], selected: 'not true not false')
    expected = ['weather', 'terminator']
    assert_equal expected, Agent.search(s.options).order(name: :asc).collect(&:agentname)

    s = PlayAgentsSearch.new(user, selected_ids: [agents(:terminator).id], selected: 'true')
    expected = ['terminator']
    assert_equal expected, Agent.search(s.options).order(name: :asc).collect(&:agentname)

    s = PlayAgentsSearch.new(user, selected_ids: [agents(:terminator).id], selected: 'false')
    expected = ['weather']
    assert_equal expected, Agent.search(s.options).order(name: :asc).collect(&:agentname)
  end


  test "Search filter_owner params" do
    user = users(:admin)

    s = PlayAgentsSearch.new(user, filter_owner: 'owned')
    expected = ['weather', 'terminator']
    assert_equal expected, Agent.search(s.options).order(name: :asc).collect(&:agentname)

    s = PlayAgentsSearch.new(user, filter_owner: 'favorites')
    assert_equal [], Agent.search(s.options).order(name: :asc).collect(&:agentname)
  end

end
