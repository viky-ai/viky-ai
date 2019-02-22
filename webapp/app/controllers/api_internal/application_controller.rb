class ApiInternal::ApplicationController < ActionController::API
  include AbstractController::Translation

  before_action :check_token
  before_action :check_format, except: [:updated]

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
