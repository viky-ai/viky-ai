class Bot < ApplicationRecord
  belongs_to :agent
  has_many :chat_sessions, dependent: :destroy

  validates :name, :endpoint, presence: true

  def self.accessible_bots(user)
    agents = Agent
               .joins(:memberships)
               .where('user_id = ? OR visibility = ?', user.id, Agent.visibilities[:is_public])
               .distinct
    agents.collect_concat { |agent| agent.accessible_bots(user) }
  end

  def send_start(session_id)
    parameters = {
      session_id: session_id
    }
    post("start", parameters)
  end

  def send_user_text(session_id, text)
    parameters = {
      user_statement: {
        type: 'says',
        text: text
      }
    }
    post("sessions/#{session_id}/user_actions", parameters)
  end

  def send_user_payload(session_id, payload)
    parameters = {
      user_statement: {
        type: 'click',
        payload: payload
      }
    }
    post("sessions/#{session_id}/user_actions", parameters)
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
      http.use_ssl = (uri.scheme == "https")
      out = http.post(uri.path, parameters.to_json, json_headers)
      Rails.logger.info "  | Completed #{out.code}"
      {
        status: out.code,
        body: out.body
      }
    end

end
