module ConsoleHelper

  def solution_debug_link(body)
    slug = body["errors_javascript"]["solution_location"]["slug"]

    parent_id = body["errors_javascript"]["solution_location"]["id"]
    position = body["errors_javascript"]["solution_location"]["position"]
    line_number = body["errors_javascript"]["solution_location"]["line_number"]

    if slug.split('/')[2] == "interpretations"
      formulation = Formulation.find_by_position_and_intent_id(position, parent_id)
      agent = formulation.intent.agent
      href = "/agents/#{slug}?locale=#{formulation.locale}"
      href << "&debug[type]=formulations"
      href << "&debug[id]=#{formulation.id}"
    else
      list = EntitiesList.find(parent_id)
      entity = list.entities.find_by_position(position)
      agent = list.agent
      href = "/agents/#{slug}?"
      href << "search=#{entity.terms.first['term']}&" if list.entities_count > 100
      href << "debug[type]=entities"
      href << "&debug[id]=#{entity.id}"
    end
    href << "&debug[line]=#{line_number}#debug-solution"

    if current_user.can?(:show, agent)
      html  = "<a href='#{href}' class='btn btn--outline'>"
      html << "  <span class='icon icon--small'>#{icon_search}</span>"
      html << t('views.console.js_error.btn')
      html << "</a>"
      html.html_safe
    else
      return nil
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

  def highlight_explain_item(matches)
    html = []
    unless matches['interpretation_slug'].split('/').last.include? 'recursive'
      html << '<li>'
      position = matches['expression_pos']
      parent_id = matches['interpretation_id']

      if matches['interpretation_slug'].include? 'entities_list'
        list = EntitiesList.find(parent_id)
        entity = list.entities.find_by_position(position)
        agent = list.agent
        href = "/agents/#{matches['interpretation_slug']}"
        href << "?search=#{entity.terms.first['term']}" if list.entities_count > 100
        href << "#smooth-scroll-to-entity-#{entity.id}"
      else
        formulation = Formulation.find_by_position_and_intent_id(position, parent_id)
        agent = formulation.intent.agent
        href = "/agents/#{matches['interpretation_slug']}"
        href << "?locale=#{formulation.locale}"
        href << "#smooth-scroll-to-formulation-#{formulation.id}"
      end

      if current_user.can?(:show, agent)
        html << "<a href='#{href}'>"
        html << "  <em>#{matches['expression']}</em>"
        html << "  <span>#{matches['interpretation_slug']}</span>"
        html << '</a>'
      else
        html << "<span title='#{t('views.console.no_access')}'>"
        html << "  <em>#{matches['expression']}</em>"
        html << "  <span>#{matches['interpretation_slug']}</span>"
        html << '</span>'
      end
      html << '</li>'
    end
    html.join.html_safe
  end

end
