class RegistrationsController < Devise::RegistrationsController

  before_action :configure_permitted_parameters

  # GET /resource/sign_up
  def new
    if Feature.user_registration_disabled?
      redirect_to new_user_session_path, alert: t('controllers.registrations.disabled_message')
    else
      super
    else
      redirect_to new_user_session_path, alert: t('controllers.registrations.disabled_message')
    end
  end

  # POST /resource
  def create
    if Feature.user_registration_disabled?
      redirect_to new_user_session_path, alert: t('controllers.registrations.disabled_message')
    else
      super
    else
      redirect_to new_user_session_path, alert: t('controllers.registrations.disabled_message')
    end
  end

  # Disable edit/update/destroy
  def edit
    raise ActionController::RoutingError.new('Not Found')
  end

  def update
    raise ActionController::RoutingError.new('Not Found')
  end

  def destroy
    raise ActionController::RoutingError.new('Not Found')
  end


  protected

    def configure_permitted_parameters
      devise_parameter_sanitizer.permit(:sign_up, keys: [:username, :email, :password])
    end

    def after_sign_up_path_for(resource)
      "/users/sign_in"
    end

    def after_inactive_sign_up_path_for(resource)
      "/users/sign_in"
    end

end
