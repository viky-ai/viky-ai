require 'net/http'

class Nls::Interpret
  JSON_HEADERS = {"Content-Type" => "application/json", "Accept" => "application/json"}
  attr_reader :options, :endpoint, :errors, :intents

  def initialize(options = {})
    @options = clean_options options
    @endpoint = "#{ENV['VOQALAPP_NLS_URL']}/api/v1"
  end

  def interpret
    uri = URI.parse "#{@endpoint}/interpret/"
    http = Net::HTTP.new uri.host, uri.port
    @intents = []
    @errors = []

    begin
      out = http.post(uri.path, options.to_json, JSON_HEADERS)
      if out.code == '200'
        @intents = JSON.parse(out.body)['intents']
      else
        @errors = JSON.parse(out.body)['errors']
      end
    rescue StandardError => sterr
      @errors << sterr.message
    end
  end


  private

    # clean parameters up to match NLS expectations
    def clean_options(options)
      (options || {}).transform_values do |v|
        v.respond_to?(:strip) ? v.strip : v
      end
      options["sentence"] = options[:sentence] || ""

      options["Accept-Language"] = options[:language] || "en-US"
      options.delete :language

      options["packages"] = [options[:agent_name]]
      options.delete :agent_name
    end

end
