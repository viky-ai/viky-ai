require 'test_helper'
require 'model_test_helper'

class AgentGraphTest < ActiveSupport::TestCase

  test 'Build the graph dependencies : single node' do
    #   A*
    agent_a = create_agent('Agent A')

    graph = AgentGraph.new(agent_a).to_graph

    assert_equal 0, graph.num_edges
    assert_equal 1, graph.num_vertices

    assert graph.has_vertex? agent_a
  end


  test 'Build the graph dependencies : single arc' do
    #   A*
    #  /
    # B
    agent_a = create_agent('Agent A')
    agent_b = create_agent('Agent B')
    assert AgentArc.create(source: agent_a, target: agent_b)
    force_reset_model_cache([agent_a, agent_b])

    graph = AgentGraph.new(agent_a).to_graph

    assert_equal 1, graph.num_edges
    assert_equal 2, graph.num_vertices

    assert graph.has_edge?(agent_a, agent_b)
    assert_not graph.has_edge?(agent_b, agent_a)
  end


  test 'Build the graph dependencies : two levels depth' do
    #   A*
    #  /
    # B
    # |
    # D
    agent_a = create_agent('Agent A')
    agent_b = create_agent('Agent B')
    agent_d = create_agent('Agent D')

    assert AgentArc.create(source: agent_a, target: agent_b)
    assert AgentArc.create(source: agent_b, target: agent_d)

    force_reset_model_cache([agent_a, agent_b, agent_d])

    graph = AgentGraph.new(agent_a).to_graph

    assert_equal 2, graph.num_edges
    assert_equal 3, graph.num_vertices

    assert graph.has_edge?(agent_a, agent_b)
    assert graph.has_edge?(agent_b, agent_d)
  end


  test 'Build the graph dependencies : two children' do
    #   A*
    #  / \
    # B   C
    agent_a = create_agent('Agent A')
    agent_b = create_agent('Agent B')
    agent_c = create_agent('Agent C')

    assert AgentArc.create(source: agent_a, target: agent_b)
    assert AgentArc.create(source: agent_a, target: agent_c)

    force_reset_model_cache([agent_a, agent_b, agent_c])

    graph = AgentGraph.new(agent_a).to_graph

    assert_equal 2, graph.num_edges
    assert_equal 3, graph.num_vertices

    assert graph.has_edge?(agent_a, agent_b)
    assert graph.has_edge?(agent_a, agent_c)
  end


  test 'Build the graph dependencies : ignoring siblings' do
    #  A  B*
    #  \ /
    #   C
    agent_a = create_agent('Agent A')
    agent_b = create_agent('Agent B')
    agent_c = create_agent('Agent C')

    assert AgentArc.create(source: agent_a, target: agent_c)
    assert AgentArc.create(source: agent_b, target: agent_c)

    force_reset_model_cache([agent_a, agent_b, agent_c])

    graph = AgentGraph.new(agent_b).to_graph

    assert_equal 1, graph.num_edges
    assert_equal 2, graph.num_vertices

    assert graph.has_edge?(agent_b, agent_c)
  end


  test 'Build the graph dependencies : ignoring parents' do
    #  A  B
    #  \ /
    #   C*
    agent_a = create_agent('Agent A')
    agent_b = create_agent('Agent B')
    agent_c = create_agent('Agent C')

    assert AgentArc.create(source: agent_a, target: agent_b)
    assert AgentArc.create(source: agent_a, target: agent_c)

    force_reset_model_cache([agent_a, agent_b, agent_c])

    graph = AgentGraph.new(agent_c).to_graph

    assert_equal 0, graph.num_edges
    assert_equal 1, graph.num_vertices

    assert graph.has_vertex? agent_c
  end


  test 'Build graph dependencies : diamond shape' do
    #   A*
    #  / \
    # B   C
    # | /
    # D
    agent_a = create_agent('Agent A')
    agent_b = create_agent('Agent B')
    agent_c = create_agent('Agent C')
    agent_d = create_agent('Agent D')

    assert assert AgentArc.create(source: agent_a, target: agent_b)
    assert assert AgentArc.create(source: agent_a, target: agent_c)
    assert assert AgentArc.create(source: agent_b, target: agent_d)
    assert assert AgentArc.create(source: agent_c, target: agent_d)

    force_reset_model_cache([agent_a, agent_b, agent_c, agent_d])

    graph = AgentGraph.new(agent_a).to_graph

    assert_equal 4, graph.num_edges
    assert_equal 4, graph.num_vertices

    assert graph.has_edge?(agent_a, agent_b)
    assert graph.has_edge?(agent_a, agent_c)
    assert graph.has_edge?(agent_b, agent_d)
    assert graph.has_edge?(agent_c, agent_d)
  end


  test 'Build the graph dependencies : custom node name' do
    #   A*
    #  / \
    # B   C
    agent_a = create_agent('Agent A')
    agent_b = create_agent('Agent B')
    agent_c = create_agent('Agent C')

    assert AgentArc.create(source: agent_a, target: agent_b)
    assert AgentArc.create(source: agent_a, target: agent_c)

    force_reset_model_cache([agent_a, agent_b, agent_c])

    graph = AgentGraph.new(agent_a).to_graph(&:slug)

    assert_equal 2, graph.num_edges
    assert_equal 3, graph.num_vertices

    assert graph.has_vertex?(agent_a.slug)
    assert graph.has_vertex?(agent_b.slug)
    assert graph.has_vertex?(agent_c.slug)
  end


  test 'Build the graph predecessors : single node' do
    #   A*
    agent_a = create_agent('Agent A')

    graph = AgentGraph.new(agent_a).to_predecessors_graph

    assert_equal 0, graph.num_edges
    assert_equal 1, graph.num_vertices

    assert graph.has_vertex? agent_a
  end


  test 'Build the graph predecessors : single arc' do
    # A
    #  \
    #   B*
    agent_a = create_agent('Agent A')
    agent_b = create_agent('Agent B')

    assert AgentArc.create(source: agent_a, target: agent_b)

    graph = AgentGraph.new(agent_b).to_predecessors_graph

    assert_equal 1, graph.num_edges
    assert_equal 2, graph.num_vertices

    assert graph.has_edge?(agent_b, agent_a)
    assert_not graph.has_edge?(agent_a, agent_b)
  end


  test 'Build the graph predecessors : two levels depth' do
    # A
    #  \
    #   B
    #   |
    #   D*
    agent_a = create_agent('Agent A')
    agent_b = create_agent('Agent B')
    agent_d = create_agent('Agent D')

    assert AgentArc.create(source: agent_a, target: agent_b)
    assert AgentArc.create(source: agent_b, target: agent_d)

    graph = AgentGraph.new(agent_d).to_predecessors_graph

    assert_equal 2, graph.num_edges
    assert_equal 3, graph.num_vertices

    assert graph.has_edge?(agent_d, agent_b)
    assert graph.has_edge?(agent_b, agent_a)
  end


  test 'Build the graph predecessors : ignoring siblings' do
    #   A
    #  / \
    # B*  C
    agent_a = create_agent('Agent A')
    agent_b = create_agent('Agent B')
    agent_c = create_agent('Agent C')

    assert AgentArc.create(source: agent_a, target: agent_b)
    assert AgentArc.create(source: agent_a, target: agent_c)

    graph = AgentGraph.new(agent_b).to_predecessors_graph

    assert_equal 1, graph.num_edges
    assert_equal 2, graph.num_vertices

    assert graph.has_edge?(agent_b, agent_a)
  end


  test 'Build the graph predecessors : two parents' do
    #  A  B
    #  \ /
    #   C*
    agent_a = create_agent('Agent A')
    agent_b = create_agent('Agent B')
    agent_c = create_agent('Agent C')

    assert AgentArc.create(source: agent_a, target: agent_c)
    assert AgentArc.create(source: agent_b, target: agent_c)

    graph = AgentGraph.new(agent_c).to_predecessors_graph

    assert_equal 2, graph.num_edges
    assert_equal 3, graph.num_vertices

    assert graph.has_edge?(agent_c, agent_a)
    assert graph.has_edge?(agent_c, agent_b)
  end


  test 'Build the graph predecessors : ignoring children' do
    #   A*
    #  / \
    # B   C
    agent_a = create_agent('Agent A')
    agent_b = create_agent('Agent B')
    agent_c = create_agent('Agent C')

    assert AgentArc.create(source: agent_a, target: agent_b)
    assert AgentArc.create(source: agent_a, target: agent_c)

    graph = AgentGraph.new(agent_a).to_predecessors_graph

    assert_equal 0, graph.num_edges
    assert_equal 1, graph.num_vertices

    assert graph.has_vertex? agent_a
  end


  test 'Build graph predecessors : diamond shape' do
    #   A
    #  / \
    # B   C
    # | /
    # D*
    agent_a = create_agent('Agent A')
    agent_b = create_agent('Agent B')
    agent_c = create_agent('Agent C')
    agent_d = create_agent('Agent D')

    assert AgentArc.create(source: agent_a, target: agent_b)
    assert AgentArc.create(source: agent_a, target: agent_c)
    assert AgentArc.create(source: agent_b, target: agent_d)
    assert AgentArc.create(source: agent_c, target: agent_d)

    graph = AgentGraph.new(agent_d).to_predecessors_graph

    assert_equal 4, graph.num_edges
    assert_equal 4, graph.num_vertices

    assert graph.has_edge?(agent_d, agent_b)
    assert graph.has_edge?(agent_d, agent_c)
    assert graph.has_edge?(agent_b, agent_a)
    assert graph.has_edge?(agent_c, agent_a)
  end


  test 'Build the graph predecessors : custom node name' do
    #  A  B
    #  \ /
    #   C*
    agent_a = create_agent('Agent A')
    agent_b = create_agent('Agent B')
    agent_c = create_agent('Agent C')

    assert AgentArc.create(source: agent_a, target: agent_c)
    assert AgentArc.create(source: agent_b, target: agent_c)

    graph = AgentGraph.new(agent_c).to_predecessors_graph(&:slug)

    assert_equal 2, graph.num_edges
    assert_equal 3, graph.num_vertices

    assert graph.has_vertex?(agent_a.slug)
    assert graph.has_vertex?(agent_b.slug)
    assert graph.has_vertex?(agent_c.slug)
  end
end
