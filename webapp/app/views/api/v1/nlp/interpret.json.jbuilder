json.interpretations @response[:body]["interpretations"].each do |interpretation|
  id = interpretation["id"]

  json.id id
  # TODO: NLP remove phantom interpretation
  json.slug interpretation["slug"]
  json.name interpretation["slug"].split('/').last

  json.score interpretation["score"]
  json.solution interpretation["solution"] unless interpretation["solution"].nil?
  json.explanation interpretation["explanation"] unless interpretation["explanation"].nil?
end
