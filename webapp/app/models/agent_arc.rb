require 'rgl/topsort' # acyclic

class AgentArc < ApplicationRecord
  belongs_to :target, foreign_key: 'target_id', class_name: 'Agent', inverse_of: :in_arcs
  belongs_to :source, foreign_key: 'source_id', class_name: 'Agent', inverse_of: :out_arcs

  validates :source, presence: true
  validates :target, uniqueness: { scope: [:source] }, presence: true
  validate :check_acyclic

  before_destroy { source.touch }
  before_destroy :remove_target_related_interpretation_aliases

  after_create_commit { sync_nlp_source }
  after_update_commit { sync_nlp_source }

  def target_related_interpretation_aliases
    InterpretationAlias.find_by_sql ["
        SELECT
          ia.id,
          ia.aliasname,
          ia.position_start,
          ia.position_end,
          ia.interpretation_id,
          ia.interpretation_aliasable_id,
          ia.interpretation_aliasable_type,
          ia.nature,
          ia.is_list,
          ia.any_enabled
        FROM interpretation_aliases AS ia
          INNER JOIN intents ON ia.interpretation_aliasable_id = intents.id
          INNER JOIN interpretations AS pr ON ia.interpretation_id = pr.id
        WHERE intents.agent_id = ?
              AND ia.interpretation_id IN (SELECT interpretations.id
                                           FROM interpretations
                                             INNER JOIN intents ON interpretations.intent_id = intents.id
                                           WHERE intents.agent_id = ?)", target_id, source_id]
  end


  private

    def remove_target_related_interpretation_aliases
      related_aliases = target_related_interpretation_aliases
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
