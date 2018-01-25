require 'rest-client'
require 'json'

module NlpRoute

  class PackageApiWrapper

    VIKY_URL = ENV.fetch("VIKYAPP_BASEURL") { 'http://localhost:3000' }
    NLP_URL =  ENV.fetch("VIKYAPP_NLP_URL") { 'http://localhost:9345' }

    # list all package from webapp
    def list_id
      puts "#{VIKY_URL}/api_internal/packages.json"
      response = RestClient.get "#{VIKY_URL}/api_internal/packages.json" , {accept: :json}
      JSON.parse(response.body)
    end

    # Get package data json from webapp
    def get_package(id)
      response = RestClient.get "#{VIKY_URL}/api_internal/packages/#{id}.json"
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
