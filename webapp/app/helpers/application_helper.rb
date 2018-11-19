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

  def highlight(words, interpretation_index)
    highlights = []
    words.each_with_index do |word, word_index|
      if word['match'].nil?
        highlights << word['word']
      else
        action = "console-explain-highlighted-word"
        target = "#highlight-explain-#{interpretation_index}-#{word_index}"
        html_match = []
        html_match << "<match class='highlight-words' data-action='#{action}' data-target='#{target}'>"
        html_match << word['word']
        html_match << "<sup>(Any)</sup>" if word['is_any'] == true
        html_match << "</match>"
        highlights << html_match.join('')
      end
    end

    highlights.join(' ').html_safe
  end


  def expression_list(matches)
    html = []
    unless matches['interpretation_slug'].split('/').last.include? 'recursive'
      html << '<li>'
      if matches['interpretation_slug'].include? 'entities_list'
        html << "<a href='/agents/#{matches['interpretation_slug']}#entity-#{matches['expression_id']}'>"
      else
        html << "<a href='/agents/#{matches['interpretation_slug']}?expression_id=#{matches['expression_id']}#interpretation-#{matches['expression_id']}'>"
      end
      html << "<em>#{matches['expression']}</em>"
      html << "<span>#{matches['interpretation_slug']}</span>"
      html << '</a>'
      html << '</li>'
    end
    html.join.html_safe
  end

end
