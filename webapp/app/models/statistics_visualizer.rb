# frozen_string_literal: true

require 'net/http'

class StatisticsVisualizer
  def endpoint
    ENV.fetch('VIKYAPP_STATISTICS_VISUALIZER_URL') do
      'http://localhost:5601'
    end
  end

  def configure
    file = "#{Rails.root}/config/kibana/kibana-conf.ndjson"
    uri = URI("#{endpoint}/kibana/api/saved_objects/_import?overwrite=true")
    request = Net::HTTP::Post.new(uri)
    request['kbn-xsrf'] = true
    form_data = [['file', File.open(file), { filename: File.basename(file) }]]
    request.set_form(form_data, 'multipart/form-data')
    Net::HTTP.start(uri.host, uri.port) do |http|
      response = http.request(request)
      if response.code != '200'
        raise "Error while saving Kibana configuration : #{response.body}"
      end
    end
    true
  end
end
