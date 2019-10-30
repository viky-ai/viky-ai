module FormulationHelper

  def rtl?(locale)
    locale == 'ar'
  end

  def interpretation_to_json(interpretation)
    JSON.generate(
      color:  "interpretation-#{interpretation.color}",
      aliasname: interpretation.interpretation_name,
      slug: interpretation.slug,
      formulation_aliasable_id: interpretation.id,
      formulation_aliasable_type: Interpretation.name,
      nature: FormulationAlias.natures.key(FormulationAlias.natures[:type_interpretation])
    )
  end

  def entities_list_to_json(entities_list)
    JSON.generate(
      color:  "entities_list-#{entities_list.color}",
      aliasname: entities_list.listname,
      slug: entities_list.slug,
      formulation_aliasable_id: entities_list.id,
      formulation_aliasable_type: EntitiesList.name,
      nature: FormulationAlias.natures.key(FormulationAlias.natures[:type_entities_list])
    )
  end

  def number_to_json
    JSON.generate({
      color:  "interpretation-black",
      aliasname: t("views.formulations.number").downcase,
      nature: FormulationAlias.natures.key(FormulationAlias.natures[:type_number])
    })
  end

  def regex_to_json
    JSON.generate({
      color: "interpretation-black",
      aliasname: t("views.formulations.regex").downcase,
      nature: FormulationAlias.natures.key(FormulationAlias.natures[:type_regex]),
      reg_exp: ""
    })
  end

  def alias_to_json_href(formulation_alias)
    ERB::Util.url_encode(alias_to_json(formulation_alias))
  end

  def alias_to_json(formulation_alias)
    if formulation_alias.type_number?
      data = {
        color: "interpretation-black",
        aliasname: formulation_alias.aliasname,
        nature: FormulationAlias.natures.key(FormulationAlias.natures[:type_number]),
        is_list: formulation_alias.is_list,
        any_enabled: formulation_alias.any_enabled,
        slug: t("views.formulations.number")
      }
    elsif formulation_alias.type_regex?
      data = {
        color: "interpretation-black",
        aliasname: formulation_alias.aliasname,
        nature: FormulationAlias.natures.key(FormulationAlias.natures[:type_regex]),
        reg_exp: formulation_alias.reg_exp,
        is_list: formulation_alias.is_list,
        any_enabled: formulation_alias.any_enabled,
        slug: t("views.formulations.regex")
      }
    else
      current_aliasable = formulation_alias.formulation_aliasable
      url = nil
      if current_user.can?(:show, current_aliasable.agent)
        if formulation_alias.type_interpretation?
          url = user_agent_interpretation_path(current_aliasable.agent.owner, current_aliasable.agent, current_aliasable)
        elsif formulation_alias.type_entities_list?
          url = user_agent_entities_list_path(current_aliasable.agent.owner, current_aliasable.agent, current_aliasable)
        end
      end
      data = {
        aliasname: formulation_alias.aliasname,
        slug: current_aliasable.slug,
        formulation_aliasable_id: current_aliasable.id,
        is_list: formulation_alias.is_list,
        any_enabled: formulation_alias.any_enabled,
        url: url,
      }
      if formulation_alias.type_interpretation?
        data[:color] = "interpretation-#{current_aliasable.color}"
        data[:formulation_aliasable_type] = Interpretation.name
        data[:nature] = FormulationAlias.natures.key(FormulationAlias.natures[:type_interpretation])
      end
      if formulation_alias.type_entities_list?
        data[:color] = "entities_list-#{current_aliasable.color}"
        data[:formulation_aliasable_type] = EntitiesList.name
        data[:nature] = FormulationAlias.natures.key(FormulationAlias.natures[:type_entities_list])
      end
    end
    data[:id] = formulation_alias.id unless formulation_alias.id.nil?

    unless formulation_alias.errors[:aliasname].empty?
      data[:aliasname_errors] = display_errors(formulation_alias, :aliasname)
    end

    unless formulation_alias.errors[:reg_exp].empty?
      data[:regex_errors] = display_errors(formulation_alias, :reg_exp)
    end

    unless formulation_alias.errors[:base].empty?
      data[:base_errors] = display_errors(formulation_alias, :base)
    end

    JSON.generate(data)
  end


  def display_expression_for_trix(formulation)
    expression = formulation.expression
    return expression if formulation.formulation_aliases.empty?

    ordered_aliases = formulation.formulation_aliases
                        .reject { |ialias| ialias._destroy }
                        .sort_by { |ialias| ialias.position_start }
    result = []
    expression.split('').each_with_index do |character, index|
      formulation_alias = ordered_aliases.first
      if formulation_alias.nil? || index < formulation_alias.position_start
        result << character
      end
      if !formulation_alias.nil? && index == formulation_alias.position_end - 1
        text = expression[formulation_alias.position_start..formulation_alias.position_end-1]
        result << "<a href=\"#{alias_to_json_href(formulation_alias)}\">#{text}</a>"
        ordered_aliases = ordered_aliases.drop(1)
      end
    end
    result.join('')
  end


  def display_expression(formulation)
    expression = formulation.expression

    result = []

    if formulation.formulation_aliases.count == 0
      result << expression
    else
      ordered_aliases = formulation.formulation_aliases.order(position_start: :asc)

      result << "<span class='formulation-resume'>"
      expression.split('').each_with_index do |character, index|
        formulation_alias = ordered_aliases.first
        if formulation_alias.nil? || index < formulation_alias.position_start
          result << character
        end
        if !formulation_alias.nil? && index == formulation_alias.position_end - 1
          if formulation_alias.type_number?
            color = "black"
            title = "Number"
          elsif formulation_alias.type_regex?
            color = "black"
            title = "Regex: #{formulation_alias.reg_exp}"
          else
            title = formulation_alias.formulation_aliasable.slug
            color = formulation_alias.formulation_aliasable.color
          end
          css_class = "formulation-resume__alias-#{color}"
          text = expression[formulation_alias.position_start..formulation_alias.position_end-1]
          result << "<span class='#{css_class}' title='#{title}'>#{text}</span>"
          ordered_aliases = ordered_aliases.drop(1)
        end
      end
      result << "</span>"
    end

    result << "&nbsp;"
    result << "&nbsp;"

    if formulation.keep_order
      result << "&nbsp;"
      result << "<span class='badge' title='#{t("views.formulations.keep_order_description")}'>"
      result << t('activerecord.attributes.formulation.keep_order')
      result << "</span>"
    end

    result << "&nbsp;"
    result << "<span class='badge' title='#{t("views.formulations.proximity.description.#{formulation.proximity}")}'>"
    result << t("views.formulations.proximity.#{formulation.proximity}")
    result << "</span>"

    if formulation.auto_solution_enabled
      result << "&nbsp;"
      result << " <span class='badge' title='#{t("views.formulations.auto_solution_description")}'>"
      result << t('activerecord.attributes.formulation.auto_solution_enabled')
      result << "</span>"
    end

    result.join('').html_safe
  end

end
