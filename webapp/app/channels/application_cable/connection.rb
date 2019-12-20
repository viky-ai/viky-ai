module ApplicationCable
  class Connection < ActionCable::Connection::Base
    identified_by :current_user

    def connect
      user = User.current_user_through_cookie(cookies)
      if user.nil?
        reject_unauthorized_connection
      else
        self.current_user = user
      end
    end

  end
end
