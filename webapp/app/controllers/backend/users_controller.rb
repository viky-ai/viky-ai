class Backend::UsersController < Backend::ApplicationController

  def index
    @search = Backend::UserSearch.new(params[:search])
    @users = User.search(@search.options)
              .page(params[:page])
  end

  def reinvite
    user = User.find(params[:id])

    unless user.nil?
      user.invite!(current_user)
      nmess = "A new invitation email has been sent to #{user.email}."
    else
      nmess = "User is unknown"
    end
    redirect_to backend_users_path, notice: nmess
  end

end
