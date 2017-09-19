class Backend::UsersController < Backend::ApplicationController

  def index
    @search = Backend::UserSearch.new(params[:search])
    @users = User.search(@search.options)
              .page(params[:page])#.per(1)
  end

end
