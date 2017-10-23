require 'net/http'

class Nls::Interpret
  include ActiveModel::Validations
  validates_with InterpretValidator

  JSON_HEADERS = {"Content-Type" => "application/json", "Accept" => "application/json"}
  attr_reader :options, :intents

  def initialize(options = {})
    @options = clean_options options
    @endpoint = "#{ENV['VOQALAPP_NLS_URL']}/api/v1"
  end

  def interpret
    return unless valid?

    uri = URI.parse "#{@endpoint}/interpret/"
    http = Net::HTTP.new uri.host, uri.port
    @intents = []

    begin
      out = http.post(uri.path, options.to_json, JSON_HEADERS)
      if out.code == '200'
        @intents = JSON.parse(out.body)['intents']
      else
        errors.add(:nls, JSON.parse(out.body)['errors'])
      end
    rescue StandardError => sterr
      errors.add(:nls, sterr.message)
    end
  end


  private

    # clean parameters up to match NLS expectations
    def clean_options(opts)
      (opts || {}).transform_values do |v|
        v.respond_to?(:strip) ? v.strip : v
      end

      opts["Accept-Language"] = opts[:language] || "en-US"
      opts.delete :language

      opts["packages"] = [opts[:id]]
      opts
    end

end
