namespace :constellation do

  desc "Generate constellation data"
  task :generate => [:environment] do |t, args|

    constellation_data = []

    agent_search = Agent.all.order(name: :asc)
    agent_search.each do |agent|

      agent_relations = []
      agent.successors.each do |successor|
        agent_relations << 'agent_' + successor.id
      end

      agent_node = {
        parent: nil,
        id: 'agent_' + agent.id,
        type: "agent",
        title: agent.name,
        slug: agent.slug,
        relations: agent_relations
      }

      constellation_data << agent_node

      agent.intents.includes(:interpretations).each do | intent |

        intent_relations = []
        intent.interpretations.each do | interpretation |
          interpretation.interpretation_aliases.each do | interpretation_alias |
            next if !interpretation_alias.type_intent?
            intent_relations << 'intent_' + interpretation_alias.intent.id
          end
        end

        intent_node = {
          parent: 'agent_' + agent.id,
          id: 'intent_' + intent.id,
          type: "intent" ,
          title: intent.intentname,
          slug: intent.slug,
          relations: intent_relations
        }

        constellation_data << intent_node

      end
    end

    File.open("tmp/constellation_data.json","w") do |f|
      f.write(JSON.pretty_generate(constellation_data))
    end

    puts "Generate constellation data done.\nsee tmp/constellation_data.json"

  end
end
