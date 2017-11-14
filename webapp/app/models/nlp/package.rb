require 'net/http'

class Nlp::Package

  JSON_HEADERS = {"Content-Type" => "application/json", "Accept" => "application/json"}

  def initialize(agent)
    @agent = agent
  end

  def destroy
    FileUtils.rm(path)
  end

  def push
    unless Rails.env.test?
      json = generate_json
      push_in_import_directory(json)

      Rails.logger.info "Started POST to NLP \"#{url}\" at #{DateTime.now}"
      uri = URI.parse(url)
      http = Net::HTTP.new(uri.host, uri.port)
      out = http.post(uri.path, json, JSON_HEADERS)
      Rails.logger.info "  Completed from NLP, status: #{out.code}"
      {
        status: out.code,
        body: JSON.parse(out.body)
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
