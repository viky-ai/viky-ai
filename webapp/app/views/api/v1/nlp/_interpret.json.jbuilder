json.warnings body["warnings"] unless body["warnings"].nil?

json.interpretations body["interpretations"].each do |interpretation|
  json.id interpretation["id"]

  # TODO: NLP remove phantom interpretation
  json.slug interpretation["slug"]
  json.name interpretation["slug"].split('/').last
  json.scope interpretation["scope"] unless interpretation["scope"].nil?

  json.score interpretation["score"]
  json.solution interpretation["solution"] unless interpretation["solution"].nil?
  json.explanation interpretation["explanation"] unless interpretation["explanation"].nil?
end
