module PlayHelper

  def play_highlight(interpreter)
    text_decorated = ""
    pos = 0
    opened_tag = 0
    closed_tag = 0
    interpretations = interpreter.agent_result.body["interpretations"]

    colors = slug_colors(interpretations)
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
        text_decorated << "<div class='highlight-pop highlight-pop--#{colors[start_highlight["slug"]]}'>"
        text_decorated << "  <div x-arrow></div>"
        text_decorated << "  <h4>Interpretation</h4>"
        text_decorated << "  <pre>#{start_highlight["slug"]}</pre>"
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


  private

  def slug_colors(interpretations)
    data = {}
    available_colors = colors.shuffle
    interpretations.collect{|i| i["slug"]}.uniq.each_with_index do |slug, i|
      if i + 1 > available_colors.size
        data[slug] = available_colors[0]
      else
        data[slug] = available_colors[i]
      end
    end
    data
  end

  def colors
    [
      "black", "red", "pink", "purple",
      "deep-purple", "indigo", "blue", "light-blue",
      "cyan", "teal", "green", "light-green",
      "lime", "amber", "orange",
      "deep-orange", "brown"
    ]
  end

end
