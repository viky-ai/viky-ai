module Common

  def nls_url
    "http://127.0.0.1:9345"
  end

  def nls_query(param)
    response  = RestClient.post(nls_url, param.to_json, content_type: :json)
    JSON.parse(response.body)
  end

end
