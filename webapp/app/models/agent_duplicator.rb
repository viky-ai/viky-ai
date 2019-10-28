class AgentDuplicator

  def initialize(agent, new_owner)
    @agent = agent
    @new_owner = new_owner
  end

  def respects_quota?
    return true unless Feature.quota_enabled?
    final = @new_owner.expressions_count + @agent.expressions_count
    final <= Quota.expressions_limit
  end

  def duplicate
    new_agent = Clowne::Cloner.call(@agent, new_owner: @new_owner) do
      include_association :memberships, Proc.new { where(rights: 'all') }
      include_association :readme
      include_association :entities_lists, clone_with: EntitiesListsCloner
      include_association :intents, clone_with: IntentsCloner
      include_association :out_arcs
      include_association :agent_regression_checks, clone_with: AgentRegressionChecksCloner

      nullify :api_token

      finalize do |source, record, params|
        AgentDuplicator.rename_agent(source, record, params[:new_owner])
        record.visibility = Agent.visibilities[:is_private]
        record.memberships.first.user = params[:new_owner]
        record.source_agent = {
          id: source.id,
          slug: source.slug
        }
        # cf. https://github.com/shrinerb/shrine/commit/3ad7ddf145bc56f564bf10b14fcc4fcea848f4a2
        unless source.image_attacher.get.nil?
          copied_file = record.image_attacher.store.upload(source.image_attacher.get)
          record.image_attacher.send(:_set, copied_file)
        end
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
      agent_count  = Agent.owned_by(new_owner).where('agentname ILIKE ?', "#{new_agentname}%").count
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
        intent.formulations.each do |formulation|
          formulation.interpretation_aliases
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
          [
            entity.terms,
            entity.auto_solution_enabled,
            entity.solution,
            entity.position,
            clone.id,
            entity.searchable_terms,
            entity.case_sensitive,
            entity.accent_sensitive
          ]
        end
        columns = [
          :terms,
          :auto_solution_enabled,
          :solution,
          :position,
          :entities_list_id,
          :searchable_terms,
          :case_sensitive,
          :accent_sensitive
        ]
        Entity.import!(columns, new_entities)
      end
    rescue ActiveRecord::RecordInvalid => e
      clone.agent.errors[:base] << e.record.errors.to_a
      raise ActiveRecord::Rollback
    end
  end
end

class IntentsCloner < Clowne::Cloner
  include_association :formulations, clone_with: InterpretationsCloner
end

class AgentRegressionChecksCloner < Clowne::Cloner
  after_persist do |origin, clone, mapper:, **|
    next if clone.expected.blank? || clone.expected['package'] != origin.agent.id

    if origin.expected['root_type'] == 'entities_list'
      origin_reference = origin.agent.entities_lists.find(clone.expected['id'])
    else
      origin_reference = origin.agent.intents.find(clone.expected['id'])
    end
    clone.expected['id'] = mapper.clone_of(origin_reference).id
    clone.expected['package'] = clone.agent.id
    clone.save
  end
end
