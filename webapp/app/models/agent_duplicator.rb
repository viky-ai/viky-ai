class AgentDuplicator

  def initialize(agent, new_owner)
    @agent = agent
    @new_owner = new_owner
  end

  def duplicate
    new_agent = Clowne::Cloner.call(@agent, new_owner: @new_owner) do
      include_association :memberships, Proc.new { where(rights: 'all') }
      include_association :readme
      include_association :entities_lists, clone_with: EntitiesListsCloner
      include_association :intents, clone_with: IntentsCloner
      include_association :out_arcs
      include_association :agent_regression_checks

      nullify :api_token

      finalize do |source, record, params|
        AgentDuplicator.rename_agent(source, record, params[:new_owner])
        record.visibility = Agent.visibilities[:is_private]
        record.memberships.first.user = params[:new_owner]
        record.source_agent = {
          id: source.id,
          slug: source.slug
        }
        source.image_attacher.copy(record.image_attacher)
      end
    end
    fix_interpretation_aliases(new_agent.to_record)
    ActiveRecord::Base.transaction do
      new_agent.persist
    end
    new_agent.to_record
  end

  private

    def self.rename_agent(source, record, new_owner)
      new_agentname    = "#{source.agentname}-copy"
      agent_count  = new_owner.agents.where('agentname ILIKE ?', "#{new_agentname}%").count
      record.agentname = agent_count.zero? ? new_agentname : "#{new_agentname}-#{agent_count}"
      record.name = agent_count.zero? ? "#{source.name} [COPY]" : "#{source.name} [COPY #{agent_count}]"
    end

    def fix_interpretation_aliases(new_agent)
      aliases_to_fix = extract_aliases_to_fix(new_agent)
      unless aliases_to_fix.empty?
        aliases_to_fix.each do |alias_to_change|
          if alias_to_change.interpretation_aliasable_type == 'Intent'
            new_agent.intents.each do |intent|
              if intent.intentname == alias_to_change.interpretation_aliasable.intentname
                alias_to_change.interpretation_aliasable = intent
              end
            end
          else
            new_agent.entities_lists.each do |entities_list|
              if entities_list.listname == alias_to_change.interpretation_aliasable.listname
                alias_to_change.interpretation_aliasable = entities_list
              end
            end
          end
        end
      end
      new_agent
    end

    def extract_aliases_to_fix(new_agent)
      aliases_to_fix = []
      new_agent.intents.each do |intent|
        intent.interpretations.each do |interpretation|
          interpretation.interpretation_aliases
            .reject { |ialias| ['type_number', 'type_regex'].include? ialias.nature }
            .select { |ialias| ialias.interpretation_aliasable.agent.id == @agent.id }
            .each { |ialias| aliases_to_fix << ialias }
        end
      end
      aliases_to_fix
    end
end

class InterpretationsCloner < Clowne::Cloner
  include_association :interpretation_aliases
end

class EntitiesListsCloner < Clowne::Cloner

  after_persist do |origin, clone|
    begin
      origin.entities.find_in_batches.each do |batch|
        new_entities = batch.map do |entity|
          [entity.terms, entity.auto_solution_enabled, entity.solution, entity.position, clone.id, entity.searchable_terms]
        end
        columns = [:terms, :auto_solution_enabled, :solution, :position, :entities_list_id, :searchable_terms]
        Entity.import!(columns, new_entities)
      end
    rescue ActiveRecord::RecordInvalid => e
      clone.agent.errors[:base] << e.record.errors.to_a
      raise ActiveRecord::Rollback
    end
  end
end

class IntentsCloner < Clowne::Cloner
  include_association :interpretations, clone_with: InterpretationsCloner
end
