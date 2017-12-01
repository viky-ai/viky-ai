json.key_format! -> (key) { key.tr(?_, ?-) }

json.id @agent.id
json.slug @agent.slug

interpretations = []
@agent.intents.each do |intent|

  intent.interpretations.each do |interpretation|
    interpretation.interpretation_aliases.where(is_list: true).order(:position_start).each do |ialias|
      interpretation_hash = {}
      interpretation_hash[:id]   = "#{ialias.intent.id}_#{ialias.id}_recursive"
      interpretation_hash[:slug] = "#{ialias.intent.slug}_#{ialias.id}_recursive"

      expressions = []

      expression = {}
      expression[:expression] = "@{#{ialias.aliasname}}"
      expression[:aliases] = []
      expression[:aliases] << {
        alias: ialias.aliasname,
        slug: "#{ialias.intent.slug}_list",
        id: "#{ialias.intent.id}_list",
        package: @agent.id
      }
      expressions << expression

      expression = {}
      expression[:expression] = "@{#{ialias.aliasname}_recursive} @{#{ialias.aliasname}}"
      expression[:aliases] = []
      expression[:aliases] << {
        alias: "#{ialias.aliasname}_recursive",
        slug: "#{ialias.intent.slug}_#{ialias.id}_recursive",
        id: "#{ialias.intent.id}_#{ialias.id}_recursive",
        package: @agent.id
      }
      expression[:aliases] << {
        alias: ialias.aliasname,
        slug: "#{ialias.intent.slug}_list",
        id: "#{ialias.intent.id}_list",
        package: @agent.id
      }
      expressions << expression

      interpretation_hash[:expressions] = expressions
      interpretations << interpretation_hash


      interpretation_hash = {}
      interpretation_hash[:id]   = "#{ialias.intent.id}_list"
      interpretation_hash[:slug] = "#{ialias.intent.slug}_list"

      expressions = []
      expression = {}
      expression[:expression] = "@{#{ialias.aliasname}_list}"
      expression[:aliases] = []
      expression[:aliases] << {
        alias: "#{ialias.aliasname}_list",
        slug: ialias.intent.slug,
        id: ialias.intent.id,
        package: @agent.id
      }
      expression[:solution] = "`({ #{ialias.aliasname}: #{ialias.aliasname}_list })`"
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
          id = !ialias.is_list ? ialias.intent.id : "#{ialias.intent.id}_#{ialias.id}_recursive"
          slug = !ialias.is_list ? ialias.intent.slug : "#{ialias.intent.slug}_#{ialias.id}_recursive"

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


    # Any Case
    if interpretation.interpretation_aliases.where(any_enabled: true).count > 0
      interpretation.interpretation_aliases.where(any_enabled: true).each do |ialias|
        any_aliasname = ialias.aliasname
        any_expression = expression.deep_dup
        old_aliases = expression[:aliases]
        any_expression[:aliases] = []
        old_aliases.each do |jsonalias|
          if jsonalias[:alias] == any_aliasname
            any_expression[:aliases] << {
              alias: any_aliasname,
              type: 'any'
            }
          else
            any_expression[:aliases] << jsonalias
          end
        end
        expressions << any_expression
      end
    end

  end

  interpretation_hash[:expressions] = expressions
  interpretations << interpretation_hash
end

json.interpretations interpretations
