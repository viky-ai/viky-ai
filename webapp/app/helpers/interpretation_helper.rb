module InterpretationHelper

  def intent_to_json(intent)
    JSON.generate({
      color:  "intent-#{intent.color}",
      aliasname: intent.intentname,
      intent_slug: intent.slug,
      intent_id: intent.id,
      nature: "type_intent"
    })
  end

  def digit_to_json()
    JSON.generate({
      color:  "intent-black",
      aliasname: t("views.interpretations.digit"),
      nature: "type_digit"
    })
  end

  def alias_to_json_href(interpretation_alias)
    ERB::Util.url_encode(alias_to_json(interpretation_alias))
  end

  def alias_to_json(interpretation_alias)
    if interpretation_alias.type_intent?
      data = {
        color: "intent-#{interpretation_alias.intent.color}",
        aliasname: interpretation_alias.aliasname,
        intent_slug: interpretation_alias.intent.slug,
        intent_id: interpretation_alias.intent.id,
        nature: 'type_intent',
        is_list: interpretation_alias.is_list
      }
    end
    if interpretation_alias.type_digit?
      data = {
        color: "intent-black",
        aliasname: interpretation_alias.aliasname,
        nature: 'type_digit',
        is_list: interpretation_alias.is_list
      }
    end
    data[:id] = interpretation_alias.id unless interpretation_alias.id.nil?
    unless interpretation_alias.errors[:aliasname].empty?
      data[:aliasname_errors] = display_errors(interpretation_alias, :aliasname)
    end

    JSON.generate(data)
  end


  def display_expression_for_trix(interpretation)
    expression = interpretation.expression
    return expression if interpretation.interpretation_aliases.empty?

    ordered_aliases = interpretation.interpretation_aliases
                        .reject { |ialias| ialias._destroy }
                        .sort_by { |ialias| ialias.position_start }
    result = []
    expression.split('').each_with_index do |character, index|
      interpretation_alias = ordered_aliases.first
      if interpretation_alias.nil? || index < interpretation_alias.position_start
        result << character
      end
      if !interpretation_alias.nil? && index == interpretation_alias.position_end - 1
        text = expression[interpretation_alias.position_start..interpretation_alias.position_end-1]
        result << "<a href=\"#{alias_to_json_href(interpretation_alias)}\">#{text}</a>"
        ordered_aliases = ordered_aliases.drop(1)
      end
    end
    result.join('')
  end


  def display_expression(interpretation)
    expression = interpretation.expression

    result = []

    if interpretation.interpretation_aliases.count == 0
      result << expression
    else
      ordered_aliases = interpretation.interpretation_aliases.order(position_start: :asc)

      result << "<span class='interpretation-resume'>"
      expression.split('').each_with_index do |character, index|
        interpretation_alias = ordered_aliases.first
        if interpretation_alias.nil? || index < interpretation_alias.position_start
          result << character
        end
        if !interpretation_alias.nil? && index == interpretation_alias.position_end - 1
          if interpretation_alias.type_intent?
            title = interpretation_alias.intent.slug
            color = interpretation_alias.intent.color
          else
            color = "black"
            title = "Digit" if interpretation_alias.type_digit?
          end
          css_class = "interpretation-resume__alias-#{color}"
          text = expression[interpretation_alias.position_start..interpretation_alias.position_end-1]
          result << "<span class='#{css_class}' title='#{title}'>#{text}</span>"
          ordered_aliases = ordered_aliases.drop(1)
        end
      end
      result << "</span>"
    end

    if interpretation.keep_order
      result << " <span class='badge'>#{t('activerecord.attributes.interpretation.keep_order')}</span>"
    end
    if interpretation.glued
      result << " <span class='badge'>#{t('activerecord.attributes.interpretation.glued')}</span>"
    end
    if interpretation.auto_solution_enabled
      result << " <span class='badge'>#{t('activerecord.attributes.interpretation.auto_solution_enabled')}</span>"
    end

    result.join('').html_safe
  end

end
