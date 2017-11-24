json.key_format! -> (key) { key.tr(?_, ?-) }

json.id @agent.id
json.slug "#{@agent.owner.username}/#{@agent.agentname}"
json.interpretations @agent.intents do |intent|
  json.id intent.id
  json.slug "#{@agent.owner.username}/#{@agent.agentname}/#{intent.intentname}"
  json.expressions intent.interpretations do |interpretation|
    json.expression interpretation.expression
    json.locale interpretation.locale unless interpretation.locale == '*'
    json.keep_order interpretation.keep_order if interpretation.keep_order
    json.glued interpretation.glued if interpretation.glued
    json.solution JSON.parse(interpretation.solution) unless interpretation.solution.blank?
  end
end
