class AgentArc < ApplicationRecord
  require 'rgl/topsort' # acyclic

  belongs_to :source, foreign_key: "source_id", class_name: "Agent"
  belongs_to :target, foreign_key: "target_id", class_name: "Agent"

  validates :source, presence: true
  validates :target, uniqueness: { scope: [:source] }, presence: true
  validate :check_acyclic

  before_destroy :remove_target_related_interpretation_aliases

  def target_related_interpretation_aliases
    InterpretationAlias.includes(:intent, :interpretation).
      where({ intents: { agent_id: self.target_id } }).
      where({ interpretations: {
        id: Interpretation.includes(:intent).where( intents: { agent_id: self.source_id }).pluck(:id)
      }})
  end


  private

    def remove_target_related_interpretation_aliases
      target_related_interpretation_aliases.destroy_all
      Nlp::Package.new(self.source).push
    end

    def check_acyclic
      return if source.nil? || target.nil?
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
