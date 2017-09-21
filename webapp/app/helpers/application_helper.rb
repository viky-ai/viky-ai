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

end
