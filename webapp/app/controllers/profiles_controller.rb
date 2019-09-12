class ProfilesController < ApplicationController
  before_action :set_profile

  def show
    @to = DateTime.now
    @from = (DateTime.now - 30.days).beginning_of_day
    @processed_requests = InterpretRequestLog.requests_over_time(@from, @to, @profile.id, 200)
    @rejected_requests = InterpretRequestLog.requests_over_time(@from, @to, @profile.id, 503)
    request_aggs = {
      "30_days": {
        from: @from,
        to: @to
      },
      "10_days": {
        from: (@to - 10.days).beginning_of_day,
        to: @to
      },
      "today": {
        from: @to.beginning_of_day,
        to: @to
      }
    }
    @agent_requests = InterpretRequestLog.requests_over_agents(@profile.id, 503, request_aggs)
    @expressions_count = Agent
                          .select('agents.id, (coalesce(COUNT(interpretations.id),0) + coalesce(SUM(entities_lists.entities_count),0)) as total')
                          .where(owner_id: @profile.id)
                          .left_outer_joins(:entities_lists, intents: :interpretations)
                          .group(:id)
                          .order('total DESC, agents.name')
                          .page(params[:expressions_page]).per(10)
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
