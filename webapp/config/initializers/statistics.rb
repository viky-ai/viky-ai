client = IndexManager.client
begin
  is_reachable = client.ping
rescue
  is_reachable = false
end
if is_reachable
  if Rails.env.development? || Rails.env.production?
    IndexManager.fetch_template_configurations.each do |template_conf|
      template = StatisticsIndexTemplate.new template_conf
      begin
        current_version = client.indices.get_template(name: template.name)[template.name]['version']
      rescue
        current_version = 'not_found'
      end
      if current_version != template.version
        Rails.logger.warn "New statistics index version : expected #{template.version} but found #{current_version}."
        Rails.logger.warn "Statistics index using template #{template.name} may need to be reindexed."
      end
    end
  end
else
  Rails.logger.warn "No statistics cluster found at #{client.transport.hosts}."
end
