client = IndexManager.client
begin
  is_reachable = client.ping
rescue
  is_reachable = false
end
if is_reachable
  if Rails.env.development? || Rails.env.production?
    IndexManager.fetch_template_configurations.each do |template_conf|
      template_name = "#{IndexManager.build_template_name_from template_conf}-active"
      begin
        template_version = client.indices.get_template(name: template_name)[template_name]['version']
      rescue
        template_version = 'not_found'
      end
      config_version = template_conf['version']
      if template_version != config_version
        Rails.logger.warn "New statistics index version : expected #{config_version} but found #{template_version}."
        Rails.logger.warn "Statistics index using template #{template_name} may need to be reindexed."
      end
    end
  end
else
  Rails.logger.warn "No statistics cluster found at #{client.transport.hosts}."
end
