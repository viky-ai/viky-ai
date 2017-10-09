class User < ApplicationRecord
  extend FriendlyId
  friendly_id :username, use: :history, slug_column: 'username'

  include ImageUploader::Attachment.new(:image)

  has_many :memberships
  has_many :agents, through: :memberships

  # Include default devise modules except :omniauthable
  devise :invitable, :database_authenticatable, :registerable,
         :recoverable, :rememberable, :trackable, :validatable,
         :confirmable, :lockable, :timeoutable

  validates :username, uniqueness: true, length: { in: 3..25 }, presence: true,
    allow_blank: false, if: Proc.new {|u| !u.invitation_token.nil? || (u.confirmation_token.nil? && u.invitation_token.nil?) }

  before_validation :clean_username

  # overload devise method to send async emails
  def send_devise_notification(notification, *args)
    devise_mailer.send(notification, self, *args).deliver_later
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
      conditions = conditions.where.not(confirmed_at: nil)
    when "not-confirmed"
      conditions = conditions.where(confirmed_at: nil)
    when "locked"
      conditions = conditions.where.not(locked_at: nil)
    end

    conditions
  end


  private

    def clean_username
      unless username.nil?
        self.username = username.parameterize(separator: '-')
      end
    end

end
