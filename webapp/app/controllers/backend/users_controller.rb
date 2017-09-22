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

end
