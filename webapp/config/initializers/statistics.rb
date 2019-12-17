require 'interpret_request_log_client'
require 'statistics_index_template'

client = InterpretRequestLogClient.long_waiting_client
begin
  is_reachable = client.ping_cluster
rescue
  is_reachable = false
end
if is_reachable
  if Rails.env.development? || Rails.env.production?
    template = StatisticsIndexTemplate.new 'active'
    current_version = client.fetch_deployed_index_version
    if current_version != template.version
      Rails.logger.warn "New statistics index version : expected #{template.version} but found #{current_version}."
      Rails.logger.warn "Statistics index using template #{template.name} may need to be reindexed."
    end
  end
else
  Rails.logger.warn "No statistics cluster found at #{client.list_cluster_hosts}."
end
