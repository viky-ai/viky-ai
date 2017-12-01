json.key_format! -> (key) { key.tr(?_, ?-) }

json.id @agent.id
json.slug @agent.slug

interpretations = []


@agent.intents.each do |intent|

  intent.interpretations.each do |interpretation|
    interpretation.interpretation_aliases.where(is_list: true).order(:position_start).each do |ialias|

      interpretation_hash = {}
      interpretation_hash[:id]   = "#{ialias.intent.id}_#{ialias.id}_list"
      interpretation_hash[:slug] = "#{ialias.intent.slug}_#{ialias.id}_list"

      expressions = []

      expression = {}
      expression[:expression] = "@{#{ialias.aliasname}}"
      expression[:aliases] = []
      expression[:aliases] << {
        alias: ialias.aliasname,
        slug: ialias.intent.slug,
        id: ialias.intent.id,
        package: @agent.id
      }
      expressions << expression

      expression = {}
      expression[:expression] = "@{#{ialias.aliasname}_list} @{#{ialias.aliasname}}"
      expression[:aliases] = []
      expression[:aliases] << {
        alias: "#{ialias.aliasname}_list",
        slug: "#{ialias.intent.slug}_#{ialias.id}_list",
        id: "#{ialias.intent.id}_#{ialias.id}_list",
        package: @agent.id
      }
      expression[:aliases] << {
        alias: ialias.aliasname,
        slug: ialias.intent.slug,
        id: ialias.intent.id,
        package: @agent.id
      }
      expressions << expression

      interpretation_hash[:expressions] = expressions
      interpretations << interpretation_hash
    end
  end



  interpretation_hash = {}
  interpretation_hash[:id]   = intent.id
  interpretation_hash[:slug] = intent.slug
  expressions = []
  intent.interpretations.each do |interpretation|
    expression = {}
    expression[:expression] = interpretation.expression_with_aliases
    unless interpretation.interpretation_aliases.empty?
      expression[:aliases] = []
      interpretation.interpretation_aliases.order(:position_start).each do |ialias|

        if ialias.type_intent?
          id = !ialias.is_list ? ialias.intent.id : "#{ialias.intent.id}_#{ialias.id}_list"
          slug = !ialias.is_list ? ialias.intent.slug : "#{ialias.intent.slug}_#{ialias.id}_list"

          expression[:aliases] << {
            alias: ialias.aliasname,
            slug: slug,
            id: id,
            package: @agent.id
          }
        end

        if ialias.type_digit?
          expression[:aliases] << {
            alias: ialias.aliasname,
            type: 'digit'
          }
        end

      end
    end
    expression[:locale] = interpretation.locale unless interpretation.locale == '*'
    expression['keep-order'] = interpretation.keep_order if interpretation.keep_order
    expression[:glued] = interpretation.glued if interpretation.glued

    if interpretation.auto_solution_enabled
      if interpretation.interpretation_aliases.empty?
        expression[:solution] = "`\"#{interpretation.expression.gsub('"', '\\"')}\"`"
      end
    else
      expression[:solution] = "`#{interpretation.solution}`" unless interpretation.solution.blank?
    end

    expressions << expression
  end
  interpretation_hash[:expressions] = expressions
  interpretations << interpretation_hash


end

json.interpretations interpretations
