class RegistrationsController < Devise::RegistrationsController

  # GET /resource/sign_up
  def new

    if Feature.user_registration_enabled?
      super
    else
      redirect_to '/', alert: t('controllers.registrations.disabled_message')
    end
  end

  # POST /resource
  def create
    if Feature.user_registration_enabled?
      super
    else
      redirect_to '/', alert: t('controllers.registrations.disabled_message')
    end
  end


end
