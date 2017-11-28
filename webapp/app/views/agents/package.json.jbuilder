json.key_format! -> (key) { key.tr(?_, ?-) }

json.id @agent.id
json.slug "#{@agent.owner.username}/#{@agent.agentname}"
json.interpretations @agent.intents do |intent|
  json.id intent.id
  json.slug "#{@agent.owner.username}/#{@agent.agentname}/#{intent.intentname}"
  json.expressions intent.interpretations do |interpretation|
    json.expression interpretation.expression_with_aliases
    unless interpretation.interpretation_aliases.empty?
      json.aliases interpretation.interpretation_aliases do |interpretation_alias|
        json.alias interpretation_alias.aliasname unless interpretation_alias.aliasname.blank?
        json.slug interpretation_alias.intent.slug
        json.id interpretation_alias.intent.id
        json.package @agent.id
      end
    end
    json.locale interpretation.locale unless interpretation.locale == '*'
    json.keep_order interpretation.keep_order if interpretation.keep_order
    json.glued interpretation.glued if interpretation.glued
    json.solution "`#{interpretation.solution}`" unless interpretation.solution.blank?
  end
end
