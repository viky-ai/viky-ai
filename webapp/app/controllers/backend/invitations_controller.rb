class Backend::InvitationsController < Devise::InvitationsController
  before_action :authenticate_admin!, only: :new
  before_action :configure_permitted_parameters

  layout :switch_layout


  # POST /resource/invitation
  def create
    self.resource = invite_resource
    resource_invited = resource.errors.empty?

    yield resource if block_given?

    if resource_invited
      if is_flashing_format? && self.resource.invitation_sent_at
        set_flash_message :notice, :send_instructions, :email => self.resource.email
      end
      redirect_to backend_users_path, notice: t('controllers.backend.application.invitation_sent', resource_email: resource.email)
    else
      respond_with_navigational(resource) { render :new }
    end
  end

  def authenticate_admin!
    unless (user_signed_in? && current_user.admin?)
      if user_signed_in?
        flash[:alert] = t('controllers.backend.application.not_admin')
        redirect_to authenticated_root_path
      else
        flash[:alert] = t('controllers.backend.application.not_signed_in')
        redirect_to new_user_session_path
      end
    end
  end

  protected

    def configure_permitted_parameters
      devise_parameter_sanitizer.permit(:accept_invitation, keys: [:username, :password])
    end

  private

    def switch_layout
      ['new', 'create'].include?(action_name) ? "application" : "devise"
    end

end
