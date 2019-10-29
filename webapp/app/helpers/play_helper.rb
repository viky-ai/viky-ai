module PlayHelper

  def play_highlight(interpreter)
    text_decorated = ""
    pos = 0
    opened_tag = 0
    closed_tag = 0
    interpretations = interpreter.result.body["interpretations"]
    colors = interpreter.result.slug_colors

    interpreter.text.each_char do |c|
      start_highlight = interpretations.find { |formulation| formulation["start_position"] == pos }
      stop_highlight  = interpretations.find { |formulation| formulation["end_position"] == pos }
      unless stop_highlight.nil?
        text_decorated << "</div>"
        closed_tag = closed_tag + 1
      end
      if start_highlight.nil?
        text_decorated << c
      else
        text_decorated << "<div class='highlight-pop highlight-pop--#{colors[start_highlight["slug"]]}' data-package='#{start_highlight['package']}' data-color='#{colors[start_highlight["slug"]]}'>"
        text_decorated << "  <div x-arrow></div>"
        text_decorated << "  <div class='highlight-pop__content'>"
        if start_highlight["slug"].include? "/interpretations/"
          if intent_has_unsuitable_list_pattern?(Intent.find(start_highlight["id"]))
            text_decorated << "  <p class='warning'>"
            text_decorated << "    <span class='icon icon--small icon--white'>#{icon_alert}</span>"
            text_decorated << "    #{t('views.play.warning.bad_list_pattern')}"
            text_decorated << "  </p>"
          end
        end
        text_decorated << "    <h4>Interpretation</h4>"
        text_decorated << "    <a href='/agents/#{start_highlight["slug"]}'>#{start_highlight["slug"]}</a>"
        text_decorated << "    <h4>Solution</h4>"
        text_decorated << "    <pre>#{JSON.pretty_generate(start_highlight["solution"])}</pre>"
        text_decorated << "  </div>"
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

  def intent_has_unsuitable_list_pattern?(intent)
    warning = false
    if intent.formulations.count == 1
      first_formulation = intent.formulations.first
      if first_formulation.formulation_aliases.count == 1
        warning = first_formulation.formulation_aliases.first.is_list?
      end
    end
    warning
  end

  def agent_has_unsuitable_list_pattern?(agent)
    if agent.intents.is_public.count == 1
      intent = agent.intents.is_public.first
      warning = intent_has_unsuitable_list_pattern?(intent)
    end
  end

end
