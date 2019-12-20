class User < ApplicationRecord
  extend FriendlyId
  friendly_id :username, use: :history, slug_column: 'username'

  include UserImageUploader::Attachment(:image)

  has_many :memberships, dependent: :destroy
  has_many :agents, through: :memberships
  has_many :favorite_agents, dependent: :destroy
  has_many :favorites, through: :favorite_agents, source: :agent
  has_many :chat_sessions, dependent: :destroy

  scope :confirmed, -> { where.not(confirmed_at: nil) }
  scope :not_confirmed, -> { where(confirmed_at: nil) }
  scope :locked, -> { where.not(locked_at: nil) }

  # Include default devise modules except :omniauthable
  devise :database_authenticatable, :registerable,
         :rememberable, :trackable, :validatable,
         :timeoutable
  devise :lockable, :recoverable, :confirmable, :invitable if Feature.email_configured?

  before_validation :clean_username

  validates :username, uniqueness: true, length: { in: 3..25 }, presence: true,
    allow_blank: false, if: Proc.new {|u| !u.invitation_token.nil? || (u.confirmation_token.nil? && u.invitation_token.nil?) }

  before_destroy :check_agents_presence, prepend: true

  after_commit do |user|
    if user.username_previously_changed?
      Agent.where(owner_id: user.id).each do |agent|
        agent.need_nlp_sync
        agent.slug = "#{user.username}/#{agent.agentname}"
        agent.save
      end
    end
  end

  def can?(action, agent)
    return false unless [:edit, :show].include? action
    return true  if action == :show && agent.is_public?
    return false if memberships.where(agent_id: agent.id).count == 0
    return true if agent.owner_id == id

    if action == :show
      true
    else
      rights = memberships.find_by(agent_id: agent.id).rights
      action == :edit && rights == 'edit' ? true : false
    end
  end

  def owner?(agent)
    agent.owner_id == id
  end

  def name_or_username
    name.blank? ? username : name
  end

  def invitation_status
    istatus = :not_invited
    if confirmed_at.nil? && !invitation_sent_at.nil?
      cntdown = (DateTime.now.in_time_zone - invitation_sent_at)
      istatus = (cntdown > User.invite_for) ? :expired : :valid
    end
    istatus
  end

  def self.search(q = {})
    conditions = where("1 = 1")

    unless q[:query].blank?
      query = I18n.transliterate q[:query]
      conditions = conditions.where(
        "lower(email) LIKE lower(?) OR lower(username) LIKE lower(?)",
        "%#{query}%",
        "%#{query}%"
      )
    end

    case q[:sort_by]
    when "last_action"
      conditions = conditions.order(
        "current_sign_in_at desc NULLS LAST, last_sign_in_at desc NULLS LAST, created_at desc"
      )
    when "last_created"
      conditions = conditions.order("created_at desc")
    when "email"
      conditions = conditions.order("email asc")
    end

    case q[:status]
    when "confirmed"
      conditions = conditions.confirmed
    when "not-confirmed"
      conditions = conditions.not_confirmed
    when "locked"
      conditions = conditions.locked
    end

    conditions
  end

  def can_be_destroyed?
    agents.joins(:memberships).where('memberships.rights' => 'all').count.zero?
  end

  # overload devise method to send async emails
  def send_devise_notification(notification, *args)
    devise_mailer.send(notification, self, *args).deliver_later
  end

  def slug
    username
  end

  def quota_exceeded?
    return false unless Feature.quota_enabled?
    return false unless quota_enabled

    quota = Quota.expressions_limit
    expressions_count >= quota
  end

  def entities_count
    EntitiesList.joins(:agent).where('agents.owner_id = ?', id).sum(:entities_count)
  end

  def formulations_count
    Formulation.joins(interpretation: :agent).where('agents.owner_id = ?', id).count
  end

  def expressions_count
    entities_count + formulations_count
  end

  def agents_by_expressions_count
    selection = <<-SQL
    (
      SELECT
        agents.id, (i.total + e.total) AS total
      FROM agents
      JOIN
      (
        SELECT
          agents.id as agent_id, coalesce(SUM(entities_lists.entities_count),0) as total
        FROM agents
        LEFT OUTER JOIN entities_lists
        ON entities_lists.agent_id = agents.id
        GROUP BY agents.id
      ) AS e
      ON agents.id = e.agent_id
      JOIN
      (
        SELECT
          agents.id as agent_id, COUNT(formulations.id) as total
        FROM agents
        LEFT OUTER JOIN interpretations
        ON interpretations.agent_id = agents.id
        LEFT OUTER JOIN formulations
        ON formulations.interpretation_id = interpretations.id
        GROUP BY agents.id
      ) AS i
    ON e.agent_id = i.agent_id
    WHERE agents.owner_id = '#{id}'
    ) as agents
    SQL

    Agent.from(selection).order("total DESC")
  end


  def self.expressions_count_per_owners
    selection = <<-SQL
      (
        SELECT
          DISTINCT agents.owner_id as id, (i.total + e.total) AS total
        FROM agents
        JOIN
        (
          SELECT
            agents.owner_id as id, coalesce(SUM(entities_lists.entities_count),0) as total
          FROM agents
          LEFT OUTER JOIN entities_lists
          ON entities_lists.agent_id = agents.id
          GROUP BY agents.owner_id
        ) AS e
        ON agents.owner_id = e.id
        JOIN
        (
          SELECT
            agents.owner_id as id, COUNT(formulations.id) as total
          FROM agents
          LEFT OUTER JOIN interpretations
          ON interpretations.agent_id = agents.id
          LEFT OUTER JOIN formulations
          ON formulations.interpretation_id = interpretations.id
          GROUP BY agents.owner_id
        ) AS i
        ON e.id = i.id
      ) as users
    SQL

    User.from(selection).order("total DESC")
  end

  def self.current_user_through_cookie(cookie)
    impersonated_user = User.find_by_id(cookie.signed[:impersonated_user_id])
    if impersonated_user.nil?
      user = User.find_by(id: cookie.signed['user_id'])
    else
      user = impersonated_user
    end
    if user && cookie.signed['user_expires_at'] > Time.now
      user
    else
      nil
    end
  end


  private

    def clean_username
      unless username.nil?
        self.username = username.parameterize(separator: '-')
      end
    end

    def check_agents_presence
      unless can_be_destroyed?
        errors.add(:base, I18n.t('errors.user.delete.agents_presence'))
        throw(:abort)
      end
    end
end
