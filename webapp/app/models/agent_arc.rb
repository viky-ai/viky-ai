class AgentArc < ApplicationRecord
  belongs_to :source, foreign_key: "source_id", class_name: "Agent"
  belongs_to :target, foreign_key: "target_id", class_name: "Agent"

  require 'rgl/topsort' # acyclic

  validates :target, uniqueness: { scope: [:source] }
  validate :check_acyclic


  private
    def check_acyclic
      src = source.list_out_arcs
      dst = target.list_out_arcs
      graph = RGL::DirectedAdjacencyGraph.new
      (src+dst).each do |(src, dst)|
        graph.add_edge(src, dst)
      end
      graph.add_edge(source, target)
      errors.add(:cycle, I18n.t('errors.agent_arc.cycle_detected')) unless graph.acyclic?
    end
end
