class User < ApplicationRecord
  extend FriendlyId
  friendly_id :username, use: :history, slug_column: 'username'

  include UserImageUploader::Attachment.new(:image)

  has_many :memberships
  has_many :agents, through: :memberships
  has_many :favorite_agents, dependent: :destroy
  has_many :favorites, through: :favorite_agents, source: :agent

  scope :confirmed, -> { where.not(confirmed_at: nil) }
  scope :not_confirmed, -> { where(confirmed_at: nil) }
  scope :locked, -> { where.not(locked_at: nil) }

  serialize :ui_state, JSON

  # Include default devise modules except :omniauthable
  devise :invitable, :database_authenticatable, :registerable,
         :recoverable, :rememberable, :trackable, :validatable,
         :confirmable, :lockable, :timeoutable

  validates :username, uniqueness: true, length: { in: 3..25 }, presence: true,
    allow_blank: false, if: Proc.new {|u| !u.invitation_token.nil? || (u.confirmation_token.nil? && u.invitation_token.nil?) }

  before_validation :clean_username
  before_destroy :check_agents_presence

  def can?(action, agent)
    return false unless [:edit, :show].include? action
    return true if action == :show && agent.is_public?
    return false if memberships.where(agent_id: agent.id).count == 0
    return true if agent.owner.id == id

    if action == :show
      true
    else
      rights = memberships.find_by(agent_id: agent.id).rights
      action == :edit && rights == 'edit' ? true : false
    end
  end

  def owner?(agent)
    agent.owner.id == id
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

    conditions = conditions.where("email LIKE ?", "%#{q[:email]}%")

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
    agents.includes(:memberships).where('memberships.rights' => 'all').count == 0
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
