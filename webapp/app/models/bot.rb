class Bot < ApplicationRecord
  belongs_to :agent
  validates :name, :endpoint, presence: true


  def send_start(session_id)
    parameters = {
      session_id: session_id
    }
    post("start", parameters)
  end

  def send_user_statement(session_id, text)
    parameters = {
      session_id: session_id,
      user_statement: text
    }
    post("user_statements", parameters)
  end


  private

    def post(method, parameters)
      json_headers = {
        "Content-Type" => "application/json",
        "Accept" => "application/json"
      }
      url = "#{endpoint}/#{method}"

      Rails.logger.info "  | Started POST: #{url} at #{Time.now}"
      Rails.logger.info "  | Parameters: #{parameters}"
      uri = URI.parse(url)
      http = Net::HTTP.new(uri.host, uri.port)
      out = http.post(uri.path, parameters.to_json, json_headers)
      Rails.logger.info "  | Completed #{out.code}"
      {
        status: out.code,
        body: JSON.parse(out.body)
      }
    end

end
