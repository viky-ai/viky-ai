module InterpretationHelper

  def display_expression_for_trix(interpretation)
    expression = interpretation.expression
    return expression if interpretation.interpretation_aliases.size == 0

    ordered_aliases = interpretation.interpretation_aliases.sort_by {|ialias| ialias.position_start}
    result = []
    expression.split('').each_with_index do |character, index|
      interpretation_alias = ordered_aliases.first
      if interpretation_alias.nil? || index < interpretation_alias.position_start
        result << character
      end
      if !interpretation_alias.nil? && index == interpretation_alias.position_end - 1
        data = {
          color: "intent-#{interpretation_alias.intent.color}",
          name: interpretation_alias.aliasname,
          slug: interpretation_alias.intent.slug,
          intent_id: interpretation_alias.intent.id,
          aliasname_errors: display_errors(interpretation_alias, :aliasname)
        }
        data[:alias_id] = interpretation_alias.id unless interpretation_alias.id.nil?
        unless interpretation_alias.errors[:aliasname].empty?
          data[:aliasname_errors] = display_errors(interpretation_alias, :aliasname)
        end
        text = expression[interpretation_alias.position_start..interpretation_alias.position_end-1]
        result << "<a href=\"#{ERB::Util.url_encode(JSON.generate(data))}\">#{text}</a>"
        ordered_aliases = ordered_aliases.drop(1)
      end
    end

    result.join('')
  end


  def display_expression(interpretation)
    expression = interpretation.expression
    return expression if interpretation.interpretation_aliases.count == 0

    ordered_aliases = interpretation.interpretation_aliases.order(position_start: :asc)
    result = []
    result << "<span class='interpretation-resume'>"
    expression.split('').each_with_index do |character, index|
      interpretation_alias = ordered_aliases.first
      if interpretation_alias.nil? || index < interpretation_alias.position_start
        result << character
      end
      if !interpretation_alias.nil? && index == interpretation_alias.position_end - 1
        slug = interpretation_alias.intent.slug
        css_class = "interpretation-resume__alias-#{interpretation_alias.intent.color}"
        text = expression[interpretation_alias.position_start..interpretation_alias.position_end-1]
        result << "<span class='#{css_class}' title='#{slug}'>#{text}</span>"
        ordered_aliases = ordered_aliases.drop(1)
      end
    end
    result << "</span>"

    if interpretation.keep_order
      result << " <span class='badge'>#{t('activerecord.attributes.interpretation.keep_order')}</span>"
    end
    if interpretation.glued
      result << " <span class='badge'>#{t('activerecord.attributes.interpretation.glued')}</span>"
    end

    result.join('').html_safe
  end

end
