module ApplicationCable
  class Connection < ActionCable::Connection::Base
    identified_by :current_user

    def connect
      self.current_user = find_verified_user
    end

    protected

      def find_verified_user
        impersonated_user = User.find_by_id(cookies.signed[:impersonated_user_id])
        if impersonated_user.nil?
          verified_user = User.find_by(id: cookies.signed['user_id'])
        else
          verified_user = impersonated_user
        end

        if verified_user && cookies.signed['user_expires_at'] > Time.now
          verified_user
        else
          reject_unauthorized_connection
        end
      end

  end
end
