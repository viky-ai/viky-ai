json.key_format! -> (key) { key.tr(?_, ?-) }

json.id @agent.id
json.slug @agent.slug
json.interpretations @agent.intents do |intent|
  json.id intent.id
  json.slug intent.slug
  json.expressions intent.interpretations do |interpretation|
    json.expression interpretation.expression_with_aliases
    unless interpretation.interpretation_aliases.empty?
      json.aliases interpretation.interpretation_aliases do |interpretation_alias|

        if interpretation_alias.type_intent?
          json.alias interpretation_alias.aliasname
          json.slug interpretation_alias.intent.slug
          json.id interpretation_alias.intent.id
          json.package @agent.id
        end

        if interpretation_alias.type_digit?
          json.alias interpretation_alias.aliasname
          json.type 'digit'
        end

        if interpretation_alias.type_any?
          json.alias interpretation_alias.aliasname
          json.type 'any'
        end

      end
    end
    json.locale interpretation.locale unless interpretation.locale == '*'
    json.keep_order interpretation.keep_order if interpretation.keep_order
    json.glued interpretation.glued if interpretation.glued
    json.solution "`#{interpretation.solution}`" unless interpretation.solution.blank?
  end
end
