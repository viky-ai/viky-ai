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

  def self.ping(endpoint)
    ping_failed = true

    url = "#{endpoint}/ping"
    begin
      response = RestClient::Request.execute(
        method: :get,
        url: url,
        headers: { accept: :json },
        timeout: 5
      )
      if response.code == 200
        ping_failed = false
        message = "Successful ping"
      else
        message = response.body
      end
    rescue Exception => e
      message = e.message
    rescue RestClient::ExceptionWithResponse => e
      message = e.message.body
    end

    if ping_failed
      [false, "Ping failed: " + message]
    else
      [true, "Successful ping"]
    end
  end

  def ping
    Bot.ping(endpoint)
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
        payload: JSON.parse(payload)
      }
    }
    Rails.logger.info parameters
    post("sessions/#{session_id}/user_actions", parameters)
  end


  private

    def post(method, parameters)
      url = "#{endpoint}/#{method}"
      RestClient::Request.execute(
        method: :post,
        url: url,
        payload: parameters.to_json,
        headers: { content_type: :json, accept: :json }
      )
    end

end
