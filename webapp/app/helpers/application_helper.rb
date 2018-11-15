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

  def highlight(words)
    highlights = []
    words.each_with_index do |word, position|
      if word['match'].nil?
      highlights << word['word']
    else
      highlight_class = "highlight-words"
      if !word['is_any'].nil? && word['is_any']
        highlight_class << " is_any"
      end
      highlights << "<match class='#{highlight_class}' id='match_#{position}''>#{word['word']}</match>"
    end
    end

    highlights.join(' ').html_safe
  end


  def expression_list(matches)
    html = []
    if !matches['interpretation_slug'].split("/").last.include? "recursive"
      html << "<li>"
      if matches['interpretation_slug'].include? "entities_list"
        html << "<a href='/agents/#{matches['interpretation_slug']}#entity-#{matches['expression_id']}'>"
      else
        html << "<a href='/agents/#{matches['interpretation_slug']}?expression_id=#{matches['expression_id']}#interpretation-#{matches['expression_id']}'>"
      end
      html << "<span class='matched-list__item__name'>#{matches['expression']}</span>"
      html << "<span class='matched-list__item__desc'>#{matches['interpretation_slug']}</span>"
      html << "</a>"
      html << "</li>"
    end
    html.join.html_safe
  end

end
