client = InterpretRequestLogClient.long_waiting_client
begin
  is_reachable = client.ping_cluster
rescue
  is_reachable = false
end
if is_reachable
  if Rails.env.development? || Rails.env.production?
    template_conf = IndexManager.template_configuration
    template = StatisticsIndexTemplate.new template_conf
    current_version = client.fetch_deployed_index_version template
    if current_version != template.version
      Rails.logger.warn "New statistics index version : expected #{template.version} but found #{current_version}."
      Rails.logger.warn "Statistics index using template #{template.name} may need to be reindexed."
    end
  end
else
  Rails.logger.warn "No statistics cluster found at #{client.list_cluster_hosts}."
end
