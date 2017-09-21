module ApplicationHelper


  def display_errors(model_instance, field)
    html = []
    if model_instance.errors[field].any?
      html << "<div class='help help--error'>"
      html << "<span class='icon icon--x-small'>"
      html << icon_alert
      html << "</span> "

      model_instance.errors[field].each do |error|
        html << t("activerecord.attributes.#{model_instance.class.to_s.underscore}.#{field}")
        html << " "
        html << error
      end

      html << "</div>"
    end
    return html.join.html_safe
  end

  def authenticate_admin!
    unless (user_signed_in? && current_user.admin?)
      if user_signed_in?
        flash[:alert] = t('controllers.backend.application.not_admin')
        redirect_to root_path
      else
        flash[:alert] = t('controllers.backend.application.not_signed_in')
        redirect_to new_user_session_path
      end
    end
  end

end
