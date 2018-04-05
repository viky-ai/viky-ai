class AgentDuplicator

  def self.duplicate(agent, new_owner)
    Clowne::Cloner.call(agent, new_owner: new_owner) do
      include_association :memberships, Proc.new { where(rights: 'all') }

      nullify :api_token

      finalize do |source, record, params|
        AgentDuplicator.rename_agent(source, record, params[:new_owner])
        record.visibility = Agent.visibilities[:is_private]
        record.memberships.first.user = params[:new_owner]
      end
    end
  end

  private

    def self.rename_agent(source, record, new_owner)
      new_agentname    = "#{source.agentname}_copy"
      agent_count  = new_owner.agents.where('agentname ILIKE ?', "#{new_agentname}%").count
      record.agentname = agent_count.zero? ? new_agentname : "#{new_agentname}_#{agent_count}"
      record.name = agent_count.zero? ? "#{source.name} [COPY]" : "#{source.name} [COPY_#{agent_count}]"
    end
end
