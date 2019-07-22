module PlayHelper

  def play_highlight(interpreter)
    text_decorated = ""
    pos = 0
    interpretations = interpreter.agent_result.body["interpretations"]
    interpreter.text.each_char do |c|
      start_highlight = interpretations.find { |interpretation| interpretation["start_position"] == pos }
      stop_highlight  = interpretations.find { |interpretation| interpretation["end_position"] == pos }
      text_decorated << "</span>" unless stop_highlight.nil?
      if start_highlight.nil?
        text_decorated << c
      else
        text_decorated << "<span class='badge badge--success'>#{c}"
      end
      pos = pos + 1
    end
    text_decorated.html_safe
  end

end
