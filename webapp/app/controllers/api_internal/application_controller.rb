class ApiInternal::ApplicationController < ApplicationController
  skip_before_action :authenticate_user!
  before_action :check_token, :check_format
  
  protected

    def check_format
      unless params[:format] == 'json' || request.headers["Accept"] =~ /json/
        render json: {message: t('controllers.api.format_not_acceptable')}, status: 406 and return
      end
    end

    def check_token
      unless request.headers["Access-Token"] == ENV.fetch("VIKYAPP_INTERNAL_API_TOKEN") { 'Uq6ez5IUdd' }
        render json: {message: t('controllers.api.access_denied')}, status: 401 and return
      end
    end
end
