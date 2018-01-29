require 'rest-client'
require 'json'

module NlpRoute

  class PackageApiWrapper

    VIKY_URL = ENV.fetch("VIKYAPP_BASEURL") { 'http://localhost:3000' }
    NLP_URL =  ENV.fetch("VIKYAPP_NLP_URL") { 'http://localhost:9345' }
    VIKYAPP_TOKEN = ENV.fetch("VIKYAPP_TOKEN") { 'Uq6ez5IUdd' }

    # list all package from webapp
    def list_id
      puts "#{VIKY_URL}/api_internal/packages.json"
      response = RestClient.get "#{VIKY_URL}/api_internal/packages.json" , { content_type: :json, 'Access-Token' => VIKYAPP_TOKEN }
      puts response;
      JSON.parse(response.body)
    end

    # Get package data json from webapp
    def get_package(id)
      response = RestClient.get "#{VIKY_URL}/api_internal/packages/#{id}.json" , { content_type: :json, 'Access-Token' => 'VIKYAPP_TOKEN' }
      JSON.parse(response.body)
    end

    # Send package to NLP
    def update_or_create(id, package)
      puts "Update: #{NLP_URL}/packages/#{id}"
      RestClient.post "#{NLP_URL}/packages/#{id}", package.to_json
      true
    end

    # delete package to NLP
    def delete(id)
      puts "Delete: #{NLP_URL}/packages/#{id}"
      RestClient.delete "#{NLP_URL}/packages/#{id}"
      true
    end

  end

end
