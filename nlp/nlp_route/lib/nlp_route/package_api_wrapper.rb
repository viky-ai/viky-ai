# frozen_string_literal: true

require 'rest-client'
require 'json'

module NlpRoute
  class PackageApiWrapper
    VIKY_URL = ENV.fetch('VIKYAPP_BASEURL') { 'http://localhost:3000' }
    NLP_URL =  ENV.fetch('VIKYAPP_NLP_URL') { 'http://localhost:9345' }
    VIKYAPP_INTERNAL_API_TOKEN = ENV.fetch('VIKYAPP_INTERNAL_API_TOKEN') { 'Uq6ez5IUdd' }
    INIT_TIMEOUT = ENV.fetch('VIKYAPP_NLP_INIT_TIMEOUT') { "#{60_000}" }

    # list all package from webapp
    def list_id
      puts " + PackageApiWrapper GET  #{VIKY_URL}/api_internal/packages.json"
      response = RestClient.get "#{VIKY_URL}/api_internal/packages.json", content_type: :json, 'Access-Token' => VIKYAPP_INTERNAL_API_TOKEN
      JSON.parse(response.body)
    end

    # Get package data json from webapp
    def get_package(id)
      puts " + PackageApiWrapper GET  #{VIKY_URL}/api_internal/packages/#{id}.json"
      response = RestClient.get "#{VIKY_URL}/api_internal/packages/#{id}.json", content_type: :json, 'Access-Token' => VIKYAPP_INTERNAL_API_TOKEN
      { version: response.headers[:etag], data: response.body }
    end

    def notify_updated_package(id, payload)
      puts " + PackageApiWrapper POST #{VIKY_URL}/api_internal/packages/#{id}/updated"
      RestClient.post "#{VIKY_URL}/api_internal/packages/#{id}/updated", payload, 'Access-Token' => VIKYAPP_INTERNAL_API_TOKEN
    end

    # Send package to NLP
    def update_or_create(id, package)
      puts " + PackageApiWrapper POST #{NLP_URL}/packages/#{id}"
      RestClient::Request.execute(
        url: "#{NLP_URL}/packages/#{id}?timeout=#{INIT_TIMEOUT}",
        method: :post,
        read_timeout: (INIT_TIMEOUT.to_i + 100),
        payload: package[:data]
      )
    end

    # delete package to NLP
    def delete(id)
      puts " + PackageApiWrapper DELETE #{NLP_URL}/packages/#{id}"
      RestClient.delete "#{NLP_URL}/packages/#{id}"
      true
    end

    def set_ready
      puts " + PackageApiWrapper POST #{NLP_URL}/ready"
      RestClient.post "#{NLP_URL}/ready", nil
    end
  end
end
