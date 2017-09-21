class Backend::InvitationsController < Devise::InvitationsController
  before_action :authenticate_admin!, only: :new
  layout :switch_layout

  def authenticate_admin!
    unless (user_signed_in? && current_user.admin?)
      if user_signed_in?
        flash[:alert] = t('controllers.backend.application.not_admin')
        redirect_to root_path
      else
        flash[:alert] = t('controllers.backend.application.not_signed_in')
        redirect_to new_user_session_path
      end
    end
  end


  private

    def switch_layout
      ['new', 'create'].include?(action_name) ? "application" : "devise"
    end

end
