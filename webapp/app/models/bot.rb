class Bot < ApplicationRecord
  belongs_to :agent, touch: true
  has_many :chat_sessions, dependent: :destroy

  validates :name, :endpoint, presence: true

  def self.search(q = {})
    conditions = self.accessible_bots(User.find(q[:user_id]))
    if q[:query].present?
      conditions = conditions.where(
        'lower(name) LIKE lower(?)',
        "%#{q[:query]}%"
      )
    end
    if q[:filter_wip]
      conditions = conditions.where(wip_enabled: false)
    end
    conditions
  end

  def self.accessible_bots(user)
    ids  = Bot.distinct.joins(agent: :memberships)
              .where(memberships: { user_id: user.id, rights: [:all, :edit] })
              .ids

    ids << Bot.distinct.joins(agent: :memberships)
              .where(memberships: { user_id: user.id, rights: [:show] })
              .where(wip_enabled: false).ids

    ids << Bot.distinct.joins(:agent)
              .where(agents: { visibility: Agent.visibilities[:is_public] })
              .where(wip_enabled: false).ids

    Bot.where(id: ids.flatten.uniq)
  end

  def self.sort_by_last_statement(bots, user)
    bot_ids = bots.collect(&:id)
    Bot
      .select('bots.*', 'latest_session.session_updated_at')
      .from(
        ChatSession
          .select(:bot_id, 'max(updated_at) AS session_updated_at')
          .where(user: user)
          .group(:bot_id)
          .unscope(:order),
        :latest_session
      )
      .joins('INNER JOIN bots ON "bots".id = "latest_session".bot_id')
      .where(id: bot_ids)
      .order('"latest_session".session_updated_at DESC')
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
    rescue StandardError => e
      message = e.message
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

  def send_start(session_id, user)
    parameters = {
      session_id: session_id,
      user: {
        name: user.name.present? ? user.name : user.username
      }
    }
    post("start", parameters)
  end

  def send_user_text(session_id, text)
    parameters = {
      user_action: {
        type: 'says',
        text: text
      }
    }
    post("sessions/#{session_id}/user_actions", parameters)
  end

  def send_user_payload(session_id, payload)
    parameters = {
      user_action: {
        type: 'click',
        payload: JSON.parse(payload)
      }
    }
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
