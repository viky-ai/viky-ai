# Hack : test failed because we write more than 10 times in the response.stream
# while the get method does not read it.
# The stream as a limit of 10 so it become full and the test hangs forever.
#
# See :
# https://github.com/rails/rails/pull/31938
# https://github.com/rails/rails/commit/a640da454fdb9cd8806a1b6bd98c2da93f1b53b9
module ::ActionController::Live
  remove_method :new_controller_thread
  def new_controller_thread
    Thread.new {
      t2 = Thread.current
      t2.abort_on_exception = true
      yield
    }
  end
end
