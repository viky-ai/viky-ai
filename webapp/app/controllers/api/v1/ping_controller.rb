class Api::V1::PingController < Api::V1::ApplicationController
  skip_before_action :verify_authenticity_token

  def ping
    head :ok
  end

end
