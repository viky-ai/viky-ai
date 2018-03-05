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
      puts "#{VIKY_URL}/api_internal/packages.json"
      response = RestClient.get "#{VIKY_URL}/api_internal/packages.json" , { content_type: :json, 'Access-Token' => VIKYAPP_INTERNAL_API_TOKEN }
      JSON.parse(response.body)
    end

    # Get package data json from webapp
    def get_package(id)
      request_with_retry "#{VIKY_URL}/api_internal/packages/#{id}.json"
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


    private
      def request_with_retry(url)
        failure_count = 0
        response = nil
        loop do
          begin
            response = RestClient.get url, { content_type: :json, 'Access-Token' => VIKYAPP_INTERNAL_API_TOKEN }
            break
          rescue RestClient::ExceptionWithResponse => e
            failure_count += 1
            return if failure_count >= FAILURE_THRESHOLD
            sleep REQUEST_WAIT * failure_count
          end
        end
        JSON.parse(response.body) unless response.nil?
      end
  end
end
