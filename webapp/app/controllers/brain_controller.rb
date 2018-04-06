class BrainController < ApplicationController
  skip_before_action :authenticate_user!, only: :index

  def index
    @display_fps = params['fps'] == 'true'
  end

end
