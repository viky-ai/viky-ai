require 'net/http'
include ActiveSupport::Benchmarkable

class Nlp::Package
  # Used to disable nlp sync if necessary via
  # Nlp::Package.sync_active = false
  class_attribute :sync_active
  self.sync_active = true

  JSON_HEADERS = {"Content-Type" => "application/json", "Accept" => "application/json"}

  def initialize(agent)
    @agent = agent
  end

  def self.push_all
    return if Rails.env.test?
    unless Nlp::Package.sync_active
      Rails.logger.info '  Skipping push_all packages to NLP because sync is deactivated'
      return
    end
    FileUtils.rm Dir.glob(File.join(Rails.root, 'import', '/*'))
    Agent.all.each do |agent|
      Nlp::Package.new(agent).push
    end
  end

  def destroy
    return if Rails.env.test?
    unless Nlp::Package.sync_active
      Rails.logger.info "  Skipping destroy of package #{@agent.id} to NLP because sync is deactivated"
      return
    end
    FileUtils.rm(path)
    Rails.logger.info "  | Started DELETE: #{url} at #{Time.now}"
    uri = URI.parse(url)
    http = Net::HTTP.new(uri.host, uri.port)
    req = Net::HTTP::Delete.new(uri.path)
    res = http.request(req)
    Rails.logger.info "  | Completed from NLP, status: #{res.code}"
  end

  def push
    return if Rails.env.test?
    unless Nlp::Package.sync_active
      Rails.logger.info "  Skipping push of package #{@agent.id} to NLP because sync is deactivated"
      return
    end
    benchmark "  NLP generate and push package: #{@agent.id}", level: :info do
      json = generate_json
      push_in_import_directory(json)

      Rails.logger.info "  | Started POST: #{url} at #{Time.now}"
      uri = URI.parse(url)
      http = Net::HTTP.new(uri.host, uri.port)
      res = http.post(uri.path, json, JSON_HEADERS)
      Rails.logger.info "  | Completed #{res.code}"
      {
        status: res.code,
        body: JSON.parse(res.body)
      }
    end
  end

  def generate_json
    JSON.pretty_generate(
      JSON.parse(
        ApplicationController.render(
          template: 'agents/package',
          assigns: { agent: @agent },
          format: :json
        )
      )
    )
  end

  def endpoint
    ENV.fetch("VOQALAPP_NLP_URL") { 'http://localhost:9345' }
  end

  def url
    "#{endpoint}/packages/#{@agent.id}"
  end

  def path
    outdirname = File.join(Rails.root, 'import')
    FileUtils.mkdir outdirname unless File.exist?(outdirname)
    File.join(outdirname, "#{@agent.id}.json")
  end


  private

    def push_in_import_directory(json)
      File.open(path, 'w') { |file| file.write json }
    end

end
