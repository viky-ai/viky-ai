json.id @agent.id
json.slug "#{@agent.owner.username}/#{@agent.agentname}"
json.interpretations @agent.intents do |intent|
  json.id intent.id
  json.slug "#{@agent.owner.username}/#{@agent.agentname}/#{intent.intentname}"
  json.expressions intent.interpretations do |interpretation|
    json.expression interpretation.expression
    json.locale interpretation.locale.gsub("_", "-")
  end
end
