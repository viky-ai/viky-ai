json.id        test.id
json.state     test.state
json.sentence  test.sentence
json.language  test.language

json.expected  do
  if test.expected.present?
    json.slug Intent.find(test.expected['id']).slug
    json.solution JSON.parse(test.expected['solution'])
  else
    ''
  end
end

got = if test.success?
        {
          slug:     Intent.find(test.got['id']).slug,
          solution: JSON.parse(test.got['solution'])
        }
      elsif test.failure? && test.got.present?
        {
          slug:     Intent.find(test.got['id']).slug,
          solution: JSON.parse(test.got['solution'])
        }
      else
        { message: t("views.agent_regression_checks.got_message.#{test.state}") }
      end
json.got got

json.state_i18n  t("views.agent_regression_checks.indicator.#{test.state}")
json.now         test.now
json.update_url  test.id.nil? ? '' : user_agent_agent_regression_check_path(owner, agent, test)
json.delete_url  test.id.nil? ? '' : user_agent_agent_regression_check_path(owner, agent, test)
json.create_url  user_agent_agent_regression_checks_path(owner, agent)
