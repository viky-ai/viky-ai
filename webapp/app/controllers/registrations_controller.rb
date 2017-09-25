class RegistrationsController < Devise::RegistrationsController

  # GET /resource/sign_up
  def new
    if FLIPPER.enabled? :user_registration
      super
    else
      redirect_to '/', alert: t('controllers.registrations.disabled_message')
    end
  end

  # POST /resource
  def create
    if FLIPPER.enabled? :user_registration
      super
    else
      redirect_to '/', alert: t('controllers.registrations.disabled_message')
    end
  end


end
