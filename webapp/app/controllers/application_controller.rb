class ApplicationController < ActionController::Base
  protect_from_forgery with: :exception
  before_action :authenticate_user!, :log_headers

  def log_headers
    logger.info request.env.select { |key, value| key.start_with?('HTTP') }
  end

end
