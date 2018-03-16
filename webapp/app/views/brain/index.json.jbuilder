final_links = []
final_nodes = []

Agent.is_public.each do |agent|

  final_nodes << {
    id: "agent_#{agent.id}",
    name: agent.name,
    name_short: truncate(agent.name, length: 20),
    slug: agent.slug,
    isAgent: true
  }

  agent.successors.each do |succ|
    if succ.is_public?
      final_links << {
        "source" => "agent_#{agent.id}",
        "target" => "agent_#{succ.id}",
        "isBetweenAgents" => true,
        "isBetweenAgentAndIntent" => false,
        "isBetweenIntents" => false
      }
    end
  end

  agent.intents.includes(:interpretations).each do |intent|
    cache_key = ['brain', 'intent', intent.id, (intent.updated_at.to_f * 1000).to_i].join('/')
    nodes, links = Rails.cache.fetch("#{cache_key}/build_internals_list_nodes") do
      inodes = []
      ilinks = []
      inodes << {
        id: "intent_#{intent.id}",
        name: intent.intentname,
        name_short: truncate(intent.intentname, length: 20),
        slug: intent.slug,
        isAgent: false
      }

      ilinks << {
        "source" => "agent_#{agent.id}",
        "target" => "intent_#{intent.id}",
        "isBetweenAgents" => false,
        "isBetweenAgentAndIntent" => true,
        "isBetweenIntents" => false
      }

      intent.interpretations.each do |interpretation|
        interpretation.interpretation_aliases.where(nature: 'type_intent').each do |interpretation_alias|
          if interpretation_alias.interpretation_aliasable.agent.is_public?
            ilinks << {
              "source" => "intent_#{intent.id}",
              "target" => "intent_#{interpretation_alias.interpretation_aliasable.id}",
              "isBetweenAgents" => false,
              "isBetweenAgentAndIntent" => false,
              "isBetweenIntents" => true
            }
          end
        end
      end

      [inodes, ilinks]
    end

    final_nodes += nodes
    final_links += links
  end

end

json.nodes final_nodes
json.links final_links
