require 'net/http'

class Nlp::Interpret
  include ActiveModel::Validations
  validates_with InterpretValidator

  JSON_HEADERS = {"Content-Type" => "application/json", "Accept" => "application/json"}
  attr_reader :options, :intents

  def initialize(options = {})
    @options = clean_options options
    @endpoint = "#{ENV['VOQALAPP_NLP_URL']}/api/v1"
  end

  def interpret
    @intents = post_to_nlp("#{@endpoint}/interpret/")['intents'] || []
    @intents.each {|i| i["name"] = i["slug"].split("/").last}
  end

  def post_to_nlp(url)
    resp = {}
    uri = URI.parse url
    http = Net::HTTP.new uri.host, uri.port
    begin
      Rails.logger.info "Started POST to NLP \"#{url}\" at #{DateTime.now}"
      Rails.logger.info "  Parameters: #{@options}"
      out = http.post(uri.path, @options.to_json, JSON_HEADERS)
      Rails.logger.info "  Completed from NLP #{out.code}"
      if out.code == '200'
        resp = JSON.parse(out.body)
      else
        errors.add(:nlp, JSON.parse(out.body)['errors'])
      end
    rescue StandardError => sterr
      errors.add(:nlp, sterr.message)
    end
    resp
  end


  private

    # clean parameters up to match NLS expectations
    def clean_options(opts)
      (opts || {}).transform_values do |v|
        v.respond_to?(:strip) ? v.strip : v
      end

      opts["Accept-Language"] = opts[:language] || "en-US"
      opts["packages"] = [opts[:id]]
      opts.select {|k,v| ["Accept-Language", "packages", "sentence"].include?(k)}
    end

end
