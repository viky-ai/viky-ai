class Backend::InvitationsController < Devise::InvitationsController
  include ApplicationHelper

  before_action :authenticate_admin!, only: :new
  layout :switch_layout


  private

  def switch_layout
    ['new', 'create'].include?(action_name) ? "application" : "devise"
  end

end
