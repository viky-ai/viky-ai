class Backend::ApplicationController < ApplicationController
  skip_before_action :authenticate_user!
  before_action :authenticate_admin!


  protected

    def authenticate_admin!
      unless (user_signed_in? && current_user.admin?)
        if user_signed_in?
          flash[:alert] = t('controllers.backend.application.not_admin')
          redirect_to agents_path
        else
          flash[:alert] = t('controllers.backend.application.not_signed_in')
          redirect_to new_user_session_path
        end
      end
    end

end
