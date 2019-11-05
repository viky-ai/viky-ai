require 'rgl/topsort' # acyclic

class AgentArc < ApplicationRecord
  belongs_to :target, foreign_key: 'target_id', class_name: 'Agent', inverse_of: :in_arcs
  belongs_to :source, foreign_key: 'source_id', class_name: 'Agent', inverse_of: :out_arcs

  alias_attribute :agent, :source
  alias_attribute :depends_on, :target

  validates :source, presence: true
  validates :target, uniqueness: { scope: [:source] }, presence: true
  validate :check_acyclic

  before_destroy { source.touch }
  before_destroy :remove_target_related_formulation_aliases

  after_create_commit { sync_nlp_source }
  after_update_commit { sync_nlp_source }

  def target_related_formulation_aliases
    FormulationAlias.find_by_sql ["
        SELECT
          ia.id,
          ia.aliasname,
          ia.position_start,
          ia.position_end,
          ia.formulation_id,
          ia.formulation_aliasable_id,
          ia.formulation_aliasable_type,
          ia.nature,
          ia.is_list,
          ia.any_enabled
        FROM formulation_aliases AS ia
          INNER JOIN interpretations ON ia.formulation_aliasable_id = interpretations.id
          INNER JOIN formulations AS pr ON ia.formulation_id = pr.id
        WHERE interpretations.agent_id = ?
              AND ia.formulation_id IN (SELECT formulations.id
                                           FROM formulations
                                             INNER JOIN interpretations ON formulations.interpretation_id = interpretations.id
                                           WHERE interpretations.agent_id = ?)", target_id, source_id]
  end


  private

    def remove_target_related_formulation_aliases
      related_aliases = target_related_formulation_aliases
      if related_aliases.present?
        related_aliases.each(&:destroy)
        source.need_nlp_sync
      end
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

    def sync_nlp_source
      source.need_nlp_sync
      source.touch
    end
end
