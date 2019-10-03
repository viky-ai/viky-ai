module ProfileGraphHelper

  def build_area_graph_data_with_quota(processed_requests, rejected_requests)
    [
      {
        name: t('views.profile.show_api_use.request_distribution.under_quota_requests'),
        data: processed_requests.map do |requests|
          [requests['key'], requests['doc_count']]
        end
      },
      {
        name: t('views.profile.show_api_use.request_distribution.over_quota_requests'),
        data: rejected_requests.map do |requests|
          [requests['key'], requests['doc_count']]
        end
      }
    ]
  end

  def build_area_graph_data_without_quota(api_requests)
    [
      {
        name: t('views.profile.show_api_use.request_distribution.all_requests'),
        data: api_requests.map do |requests|
          [requests['key'], requests['doc_count']]
        end
      }
    ]
  end

  def build_heatmap_data(data)
    data.map do |item|
      {
        name: item["key"],
        data: item["requests"]["buckets"].map do |day|
          {
            x: Date.parse(day['key_as_string']),
            y: day['doc_count']
          }
        end
      }
    end.reverse
  end

end
