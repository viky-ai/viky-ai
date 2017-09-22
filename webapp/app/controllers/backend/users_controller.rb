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
    @user = User.find(params[:id])
    render partial: 'confirm_destroy', locals: { user: @user }
  end

  def destroy
    @user = User.find(params[:id])
    @user.destroy
    redirect_to backend_users_url, notice: t('views.backend.users.destroy.message', email: @user.email)
  end

end
