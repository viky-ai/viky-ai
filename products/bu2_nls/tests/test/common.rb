module Common

  def nls_url

    listening_address = ENV['NLS_LISTENNING_ADDRESS']
    if listening_address.nil?
      listening_address = '127.0.0.1:9345'
    else
      listening_address = listening_address.gsub('0.0.0.0', '127.0.0.1')
    end

    "http://#{listening_address}/test"
  end

  def nls_query_post_by_parameters(parameters)
    #response  = RestClient.post(nls_url, param.to_json, content_type: :json)
    response  = RestClient.post(nls_url, params: parameters)
    JSON.parse(response.body)
  end

  def nls_query_get(parameters)
      response  = RestClient.get(nls_url, params: parameters)
      JSON.parse(response.body)
    end

  def nls_query_post_by_body(param)
    response  = RestClient.post(nls_url, param.to_json, content_type: :json)
    JSON.parse(response.body)
  end
end
