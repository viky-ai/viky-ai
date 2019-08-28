module PlayHelper

  def play_highlight(interpreter)
    text_decorated = ""
    pos = 0
    opened_tag = 0
    closed_tag = 0
    interpretations = interpreter.result.body["interpretations"]
    colors = interpreter.result.slug_colors

    interpreter.text.each_char do |c|
      start_highlight = interpretations.find { |interpretation| interpretation["start_position"] == pos }
      stop_highlight  = interpretations.find { |interpretation| interpretation["end_position"] == pos }
      unless stop_highlight.nil?
        text_decorated << "</div>"
        closed_tag = closed_tag + 1
      end
      if start_highlight.nil?
        text_decorated << c
      else
        text_decorated << "<div class='highlight-pop highlight-pop--#{colors[start_highlight["slug"]]}' data-package='#{start_highlight['package']}' data-color='#{colors[start_highlight["slug"]]}'>"
        text_decorated << "  <div x-arrow></div>"
        text_decorated << "  <h4>Interpretation</h4>"
        text_decorated << "  <a href='/agents/#{start_highlight["slug"]}'>#{start_highlight["slug"]}</a>"
        text_decorated << "  <h4>Solution</h4>"
        text_decorated << "  <pre>#{JSON.pretty_generate(start_highlight["solution"])}</pre>"
        text_decorated << "</div>"
        text_decorated << "<div class='highlight highlight--#{colors[start_highlight["slug"]]}'>"
        text_decorated << "#{c}"
        opened_tag = opened_tag + 1
      end
      pos = pos + 1
    end
    text_decorated << "</span>" if opened_tag - closed_tag == 1
    text_decorated.html_safe
  end

end
