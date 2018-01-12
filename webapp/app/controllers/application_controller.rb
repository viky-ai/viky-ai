class ApplicationController < ActionController::Base
  impersonates :user
  protect_from_forgery with: :exception
  before_action :authenticate_user!

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

end
