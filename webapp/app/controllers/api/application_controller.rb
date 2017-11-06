class Api::ApplicationController < ApplicationController
  skip_before_action :authenticate_user!
  before_action :check_format


  protected

    def check_format
      unless params[:format] == 'json' || request.headers["Accept"] =~ /json/
        render json: {message: t('controllers.api.format_not_acceptable')}, status: 406 and return
      end
    end
end
