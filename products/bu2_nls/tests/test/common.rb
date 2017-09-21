module Common

  def nls_url

    listening_address = ENV['NLS_LISTENNING_ADDRESS']
    if listening_address.nil?
      listening_address = '127.0.0.1:9345'
    else
      listening_address = listening_address.gsub('0.0.0.0', '127.0.0.1')
    end

    "http://#{listening_address}"
  end

  def nls_query_post(param)
    response  = RestClient.post(nls_url, param.to_json, content_type: :json)
    JSON.parse(response.body)
  end

end
