require 'rgl/topsort' # acyclic

class AgentArc < ApplicationRecord
  belongs_to :source, foreign_key: 'source_id', class_name: 'Agent', touch: true
  belongs_to :target, foreign_key: 'target_id', class_name: 'Agent'

  validates :source, presence: true
  validates :target, uniqueness: { scope: [:source] }, presence: true
  validate :check_acyclic

  before_destroy :remove_target_related_interpretation_aliases

  def target_related_interpretation_aliases
    InterpretationAlias
      .includes(:intent, :interpretation)
      .where(intents: { agent_id: target_id })
      .where(
        interpretations: {
          id: Interpretation.includes(:intent)
                .where(intents: { agent_id: source_id })
                .pluck(:id)
        }
      )
  end


  private

    def remove_target_related_interpretation_aliases
      target_related_interpretation_aliases.destroy_all
      Nlp::Package.new(source).push
    end

    def check_acyclic
      return if source.nil? || target.nil?

      src = AgentGraph.new(source).list_out_arcs
      dst = AgentGraph.new(target).list_out_arcs
      graph = RGL::DirectedAdjacencyGraph.new
      (src + dst).each { |from, to| graph.add_edge(from, to) }
      graph.add_edge(source, target)

      return if graph.acyclic?

      errors.add(:cycle, I18n.t('errors.agent_arc.cycle_detected'))
    end
end
