class ApplicationController < ActionController::Base
  impersonates :user
  protect_from_forgery with: :exception
  before_action :authenticate_user!
  skip_before_action :authenticate_user!, only: [:connexion_state]

  def connexion_state
    render partial: "connexion_state"
  end

  def access_denied
    respond_to do |format|
      format.json {
        render json: { message: t('authorisation_refused') }, status: 403
      }
      format.js {
        flash[:alert] = t('authorisation_refused')
        redirect_to agents_path
      }
      format.html{
        flash[:alert] = t('authorisation_refused')
        redirect_to agents_path
      }
    end
  end

  protected

  def after_sign_in_path_for(resource)
    "/agents"
  end

  def after_sign_out_path_for(resource)
    "/users/sign_in"
  end
end
