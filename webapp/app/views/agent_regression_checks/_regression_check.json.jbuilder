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

json.got case test.state
          when 'unknown'
            { message: t('views.agent_regression_checks.not_yet_run') }
          when 'error'
            { message: t('views.agent_regression_checks.run_error') }
          when 'running'
            { message: t('views.agent_regression_checks.running') }
          when 'success'
            {
              slug:     Intent.find(test.got['id']).slug,
              solution: JSON.parse(test.got['solution'])
            }
          else
            if test.got.present?
              {
                slug:     Intent.find(test.got['id']).slug,
                solution: JSON.parse(test.got['solution'])
              }
            else
              { message: t('views.agent_regression_checks.not_intent_found') }
            end
        end

json.state_i18n  t("views.agent_regression_checks.indicator.#{test.state}")
json.now         test.now
json.update_url  test.id.nil? ? '' : user_agent_agent_regression_check_path(owner, agent, test)
json.delete_url  test.id.nil? ? '' : user_agent_agent_regression_check_path(owner, agent, test)
json.create_url  user_agent_agent_regression_checks_path(owner, agent)
