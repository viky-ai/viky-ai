class User < ApplicationRecord
  # Include default devise modules. Others available are:
  # :omniauthable
  devise :invitable, :database_authenticatable, :registerable,
         :recoverable, :rememberable, :trackable, :validatable,
         :confirmable, :lockable, :timeoutable


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
end
