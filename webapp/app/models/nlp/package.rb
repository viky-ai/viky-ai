require 'net/http'

class Nlp::Package
  include ActiveModel::Validations
  validates_with PackageValidator

  JSON_HEADERS = {"Content-Type" => "application/json", "Accept" => "application/json"}
  attr_reader :options, :data

  def initialize(options = {})
    @options = clean_options options
    @endpoint = "#{ENV['VOQALAPP_NLP_URL']}"
  end

  def update
    ret = false
    return ret unless valid?

    @data = post_to_nlp("#{@endpoint}/packages/#{@options[:agent_id]}") || {}
    if errors.empty?
      outfilename = File.join(Rails.root, 'import', "#{@options[:agent_id]}.json")
      File.open(outfilename, 'w') { |file| file.write @data.to_json }
    end
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

    # clean parameters up to match NLP expectations
    def clean_options(opts)
      (opts || {}).transform_values do |v|
        v.respond_to?(:strip) ? v.strip : v
      end
    end

end
