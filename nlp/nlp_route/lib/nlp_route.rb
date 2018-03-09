require 'rest-client'

# Show request body on error
RestClient::ExceptionWithResponse.class_eval do

  # override message method
  def message

    full_message = (@message || default_message)
    if http_body
      begin
        full_message = full_message + "\n" + JSON.pretty_generate(JSON.parse(http_body))
      rescue
        full_message = full_message + "\n" + http_body if http_body
      end
    end

  end

end

require 'nlp_route/package_api_wrapper'
require 'nlp_route/nlp_launcher'
