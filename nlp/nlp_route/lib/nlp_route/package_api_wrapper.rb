require 'rest-client'
require 'json'

module NlpRoute

  class PackageApiWrapper

    VIKY_URL = ENV.fetch("VIKYAPP_BASEURL") { 'http://localhost:3000' }
    NLP_URL =  ENV.fetch("VIKYAPP_NLP_URL") { 'http://localhost:9345' }
    VIKYAPP_INTERNAL_API_TOKEN = ENV.fetch("VIKYAPP_INTERNAL_API_TOKEN") { 'Uq6ez5IUdd' }
    REQUEST_WAIT = 1/200 # 50ms
    FAILURE_THRESHOLD = 3

    # list all package from webapp
    def list_id
      puts " + PackageApiWrapper GET  #{VIKY_URL}/api_internal/packages.json"
      response = RestClient.get "#{VIKY_URL}/api_internal/packages.json" , { content_type: :json, 'Access-Token' => VIKYAPP_INTERNAL_API_TOKEN }
      JSON.parse(response.body)
    end

    # Get package data json from webapp
    def get_package(id)
      puts " + PackageApiWrapper GET  #{VIKY_URL}/api_internal/packages/#{id}.json"
      response = RestClient.get "#{VIKY_URL}/api_internal/packages/#{id}.json" , { content_type: :json, 'Access-Token' => VIKYAPP_INTERNAL_API_TOKEN }
      JSON.parse(response.body)
    end

    # Send package to NLP
    def update_or_create(id, package)
      puts " + PackageApiWrapper POST #{NLP_URL}/packages/#{id}"
      RestClient.post "#{NLP_URL}/packages/#{id}", package.to_json
      true
    end

    # delete package to NLP
    def delete(id)
      puts " + PackageApiWrapper DELETE #{NLP_URL}/packages/#{id}"
      RestClient.delete "#{NLP_URL}/packages/#{id}"
      true
    end

  end
end
