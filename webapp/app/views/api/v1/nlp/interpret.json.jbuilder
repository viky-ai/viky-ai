json.warnings @response[:body]["warnings"] unless @response[:body]["warnings"].nil?

json.interpretations @response[:body]["interpretations"].each do |interpretation|
  json.id interpretation["id"]

  # TODO: NLP remove phantom interpretation
  json.slug interpretation["slug"]
  json.name interpretation["slug"].split('/').last

  json.score interpretation["score"]
  json.solution interpretation["solution"] unless interpretation["solution"].nil?
  json.explanation interpretation["explanation"] unless interpretation["explanation"].nil?
end
