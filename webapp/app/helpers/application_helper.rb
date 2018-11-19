module ApplicationHelper

  def display_errors(model_instance, field)
    html = []
    if model_instance.errors[field].any?

      model_instance.errors[field].each do |error|
        html << "<div class='help help--error'>"
        html << "<span class='icon icon--x-small'>"
        html << icon_alert
        html << "</span>"
        html << " "
        html << t("activerecord.attributes.#{model_instance.class.to_s.underscore}.#{field}")
        html << " "
        html << "#{error}<br/>"
        html << "</div>"
      end

    end
    return html.join.html_safe
  end

  def doc_url
    if Rails.env.production?
      '/doc/'
    else
      'http://localhost:4567'
    end
  end

end
