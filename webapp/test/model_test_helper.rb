def create_agent(name, user=:admin)
  agent = Agent.new(
    name: name,
    agentname: name.parameterize
  )
  agent.memberships << Membership.new(user_id: users(user).id, rights: 'all')
  assert agent.save
  agent
end


def force_reset_model_cache(models)
  if models.is_a? Array
    models.each(&:reload)
  else
    models.reload
  end
end


def create_agent_regression_check_fixtures
  @regression_weather_forecast = AgentRegressionCheck.new({
    sentence: "Quel temps fera-t-il demain ?",
    language: "*",
    agent: agents(:weather),
    state: 4,
    position: 0,
    expected: {
      root_type: 'intent',
      package: agents(:weather).id,
      id: intents(:weather_forecast).id,
      solution: interpretations(:weather_forecast_tomorrow).solution.to_json.to_s
    }
  })
  @regression_weather_forecast.save!

  @regression_weather_question = AgentRegressionCheck.new({
    sentence: "What's the weather like in London?",
    language: "en",
    now: "2019-01-20 14:00:15.000000",
    agent: agents(:weather),
    state: 2,
    position: 1,
    expected: {
      root_type: 'intent',
      package: agents(:weather).id,
      id: intents(:weather_question).id,
      solution: interpretations(:weather_question_like).solution.to_json.to_s
    }
  })
  @regression_weather_question.save!

  @regression_weather_condition = AgentRegressionCheck.new({
    sentence: "Sun today",
    language: "en",
    now: "2019-01-20 14:00:15.000000",
    agent: agents(:weather),
    state: 1,
    position: 2,
    expected: {
      root_type: 'entities_list',
      package: agents(:weather).id,
      id: entities_lists(:weather_conditions).id,
      solution: entities(:weather_sunny).solution.to_json.to_s
    }
  })
  @regression_weather_condition.save!
end

def create_entities_import_fixtures
  file_path = File.join(Rails.root, 'test', 'fixtures', 'files', 'weather_condition_import.csv')
  CSV.open(file_path, "wb") do |csv|
    csv << ["terms", "auto solution", "solution"]
    csv << ["cloudy", "True", "\"{'weather': 'cloudy'}\"\n"]
  end
  
  @weather_conditions_import = EntitiesImport.new({
    file: File.open(file_path),
    mode: 'append',
    entities_list: entities_lists(:weather_conditions)
  })
  @weather_conditions_import.save!
end
