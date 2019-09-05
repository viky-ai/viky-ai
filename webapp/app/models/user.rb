class User < ApplicationRecord
  extend FriendlyId
  friendly_id :username, use: :history, slug_column: 'username'

  include UserImageUploader::Attachment.new(:image)

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

  validates :username, uniqueness: true, length: { in: 3..25 }, presence: true,
    allow_blank: false, if: Proc.new {|u| !u.invitation_token.nil? || (u.confirmation_token.nil? && u.invitation_token.nil?) }

  before_validation :clean_username
  before_destroy :check_agents_presence, prepend: true

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
    when "email"
      conditions = conditions.order("email ASC")
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
