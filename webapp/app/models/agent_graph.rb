class AgentGraph

  def initialize(agent)
    @agent = agent
  end

  def to_graph(&block)
    graph = RGL::DirectedAdjacencyGraph.new
    node = block_given? ? yield(@agent) : @agent
    graph.add_vertex(node)
    list_out_arcs(&block).each do |(src, target)|
      graph.add_edge(src, target)
    end
    graph
  end

  def list_out_arcs(&block)
    arcs_list = []
    if block_given?
      arcs_list += @agent.successors.map { |successor| [yield(@agent), yield(successor)] }
    else
      arcs_list += @agent.successors.map { |successor| [@agent, successor] }
    end
    @agent.successors.each do |successor|
      arcs_list += AgentGraph.new(successor).list_out_arcs(&block)
    end
    arcs_list
  end

  def to_predecessors_graph(&block)
    graph = RGL::DirectedAdjacencyGraph.new
    node = block_given? ? yield(@agent) : @agent
    graph.add_vertex(node)
    list = block_given? ? list_in_arcs(&block) : list_in_arcs
    list.each do |(src, target)|
      graph.add_edge(src, target)
    end
    graph
  end

  def list_in_arcs(&block)
    arcs_list = []
    if block_given?
      arcs_list += @agent.predecessors.map { |predecessor| [yield(predecessor), yield(@agent)] }
    else
      arcs_list += @agent.predecessors.map { |predecessor| [@agent, predecessor] }
    end
    @agent.predecessors.each do |predecessor|
      arcs_list += AgentGraph.new(predecessor).list_in_arcs(&block)
    end
    arcs_list
  end

  def to_graph_image(direction = :successors)
    image_name = "#{Rails.root.join('tmp')}/#{@agent.id}_#{Time.now.to_i}"
    graph = direction == :successors ? to_graph(&:slug) : to_predecessors_graph(&:slug)
    image = graph.write_to_graphic_file('svg', image_name)
    svg = File.open(image, 'rb').read
    File.delete(image, "#{image_name}.dot")
    svg
  end

end
