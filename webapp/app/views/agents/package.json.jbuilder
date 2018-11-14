json.key_format! -> (key) { key.tr(?_, ?-) }

json.id @agent.id
json.slug @agent.slug

json.interpretations @interpretations do |intent|
  json.id intent[:id]
  json.slug intent[:slug]
  json.scope intent[:scope] unless intent[:scope].blank?
  json.expressions intent[:expressions] do |interpretation|
    json.expression interpretation[:expression]
    unless interpretation[:aliases].blank?
      json.aliases interpretation[:aliases] do |ialias|
        json.alias ialias[:alias]
        json.slug ialias[:slug]  unless ialias[:slug].blank?
        json.id ialias[:id]      if ialias[:type].blank?
        json.package @agent.id   if ialias[:type].blank?
        json.type ialias[:type]  unless ialias[:type].blank?
      end
    end
    json.locale interpretation[:locale]               unless interpretation[:locale].blank?
    json.keep_order interpretation[:keep_order]       unless interpretation[:keep_order].blank?
    json.glued interpretation[:glued]                 unless interpretation[:glued].blank?
    json.glue_strength interpretation[:glue_strength] unless interpretation[:glue_strength].blank?
    json.solution interpretation[:solution]           unless interpretation[:solution].blank?
  end
end
