class Backend::ApplicationController < ApplicationController
  include ApplicationHelper

  skip_before_action :authenticate_user!
  before_action :authenticate_admin!
end
