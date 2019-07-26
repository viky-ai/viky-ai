class Backend::UsersController < Backend::ApplicationController

  def index
    @search = Backend::UserSearch.new(params[:search])
    @users = User.search(@search.options)
              .page(params[:page])
  end

  def reinvite
    user = User.find(params[:id])
    user.invite!(current_user)
    redirect_to backend_users_path, notice: t('controllers.backend.users.invitation_resent', resource_email: user.email)
  end

  def confirm_destroy
    @user = User.friendly.find(params[:id])
    render partial: 'confirm_destroy', locals: { user: @user }
  end

  def destroy
    @user = User.friendly.find(params[:id])
    if @user.destroy
      redirect_to backend_users_url, notice: t('views.backend.users.destroy.success_message', email: @user.email)
    else
      redirect_to backend_users_url, alert: t(
        'views.backend.users.destroy.errors_message',
        errors: @user.errors.full_messages.join(', ')
      )
    end
  end

  def impersonate
    user = User.find(params[:id])
    impersonate_user(user)
    cookies.signed[:impersonated_user_id] = user.id # Needed for ActionCable
    redirect_to agents_path, notice: t('views.backend.users.index.switch.success_message', email: user.email)
  end

end
