json.interpretations @response[:body]["interpretations"].each do |interpretation|
  id = interpretation["id"]
  intent = Intent.find(id)
  json.id id
  json.slug intent.slug
  json.name intent.intentname
  json.score interpretation["score"]
  json.solution interpretation["solution"] unless interpretation["solution"].nil?
  json.explanation interpretation["explanation"] unless interpretation["explanation"].nil?
end
