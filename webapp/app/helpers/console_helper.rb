module ConsoleHelper

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

  def highlight_explain_item(matches)
    html = []
    unless matches['interpretation_slug'].split('/').last.include? 'recursive'
      html << '<li>'
      if matches['interpretation_slug'].include? 'entities_list'
        href = "/agents/#{matches['interpretation_slug']}"
        href << "#smooth-scroll-to-entity-#{matches['expression_id']}"
      else
        id = matches['expression_id']
        interpretation = Interpretation.find_by_id(id)
        href = "/agents/#{matches['interpretation_slug']}"
        href << "?locale=#{interpretation.locale}"
        href << "#smooth-scroll-to-interpretation-#{id}"
      end
      html << "<a href='#{href}'>"
      html << "<em>#{matches['expression']}</em>"
      html << "<span>#{matches['interpretation_slug']}</span>"
      html << '</a>'
      html << '</li>'
    end
    html.join.html_safe
  end

end
