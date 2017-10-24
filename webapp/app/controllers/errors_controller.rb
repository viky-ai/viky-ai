class ErrorsController < ActionController::Base

  layout "marketing"

  def not_found
    @title = "404 - viky.ai"
    @message = t('errors.not_found.comment')
    render status: 404
  end

  def internal_error
    exception = env['action_dispatch.exception']
    @message = exception.message
    @application_trace = ActionDispatch::ExceptionWrapper.new(env, exception).application_trace

    @title = "500 - viky.ai"
    render status: 500
  end

end
