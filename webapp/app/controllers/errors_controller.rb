class ErrorsController < ActionController::Base

  def not_found
    @title = "404 - viky.ai"
    @message = t('errors.not_found.comment')
    render status: 404
  end

  def internal_error
    @title = "500 - viky.ai"
    render status: 500
  end

end
