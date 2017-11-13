json.intents @response[:body]["intents"].each do |intent|
  json.id   intent["id"]
  json.slug intent["slug"]
  json.name intent["slug"].split("/").last
  json.score intent["score"]
end
