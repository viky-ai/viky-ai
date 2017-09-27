class ProfilesController < ApplicationController
  before_action :set_profile

  def show
  end

  def edit
  end

  def update
    password = user_params[:password]

    if password.blank?
      data = user_without_password_params
    elsif @profile.valid_password? password
      data = user_without_password_params
    else
      data = user_params
    end

    if @profile.update(data)
      bypass_sign_in(@profile)
      redirect_to edit_profile_path
    else
      render 'edit'
    end
  end

  def confirm_destroy
    render partial: 'confirm_destroy', locals: { profile: @profile }
  end

  def destroy
    @profile.destroy
    redirect_to root_path, notice: t('views.profile.confirm_destroy.success_message')
  end


  private

    def set_profile
      @profile = current_user
    end

    def user_params
      params.require(:profile).permit(:email, :password, :name, :username, :bio, :image, :remove_image)
    end

    def user_without_password_params
      params.require(:profile).permit(:email, :name, :username, :bio, :image, :remove_image)
    end

end
