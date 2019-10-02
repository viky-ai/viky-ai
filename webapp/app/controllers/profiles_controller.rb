class ProfilesController < ApplicationController
  before_action :set_profile

  def show
    @top_agents_by_expressions_count = @profile
      .agents_by_expressions_count
      .page(params[:expressions_page])
      .per(10)

    @to = DateTime.now
    @from = (DateTime.now - 30.days).beginning_of_day

    @requests_count =  InterpretRequestReporter.new
      .with_owner(@profile.id)
      .between(@from, @to)
      .from_api
      .count

    if Feature.quota_enabled? && @profile.quota_enabled
      @requests_under_count =  InterpretRequestReporter.new
        .with_owner(@profile.id)
        .between(@from, @to)
        .under_quota
        .count

      @requests_over_count =  InterpretRequestReporter.new
        .with_owner(@profile.id)
        .between(@from, @to)
        .over_quota
        .count

      @processed_requests_over_time = InterpretRequestReporter.new
        .with_owner(@profile.id)
        .between(@from, @to)
        .under_quota
        .count_per_hours

      @rejected_requests_over_time = InterpretRequestReporter.new
        .with_owner(@profile.id)
        .between(@from, @to)
        .over_quota
        .count_per_hours

      @heapmap_under = InterpretRequestReporter.new
        .with_owner(@profile.id)
        .between(@from, @to)
        .under_quota
        .count_per_agent_and_per_day

      @heapmap_over = InterpretRequestReporter.new
        .with_owner(@profile.id)
        .between(@from, @to)
        .over_quota
        .count_per_agent_and_per_day

    else
      @api_requests_over_time = InterpretRequestReporter.new
        .with_owner(@profile.id)
        .between(@from, @to)
        .from_api
        .count_per_hours

      @api_heapmap = InterpretRequestReporter.new
        .with_owner(@profile.id)
        .between(@from, @to)
        .from_api
        .count_per_agent_and_per_day
    end
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
    if @profile.destroy
      redirect_to new_user_session_path, notice: t('views.profile.confirm_destroy.success_message')
    else
      redirect_to edit_profile_path, alert: t(
        'views.profile.confirm_destroy.errors_message',
        errors: @profile.errors.full_messages.join(', ')
      )
    end
  end

  def stop_impersonating
    stop_impersonating_user
    cookies.delete :impersonated_user_id # Needed for ActionCable
    redirect_to agents_path, notice: t('views.profile.stop_switch.success_message')
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
