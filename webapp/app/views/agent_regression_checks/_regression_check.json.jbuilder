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


if test.expected.present?
  diff_rows = []
  if test.success?
    diff_rows << {
      label: t('views.agent_regression_checks.result_slug_all'),
      value: Intent.find(test.expected['id']).slug
    }
    diff_rows << {
      label: t('views.agent_regression_checks.result_solution_all'),
      value: JSON.parse(test.expected['solution'])
    }
  elsif test.failure? && test.got.present?
    if test.expected['id'] == test.got['id']
      diff_rows << {
        label: t('views.agent_regression_checks.result_slug_all'),
        value: Intent.find(test.expected['id']).slug
      }
    else
      diff_rows << {
        label: t('views.agent_regression_checks.result_slug_expected'),
        value: Intent.find(test.expected['id']).slug
      }
      diff_rows << {
        label: t('views.agent_regression_checks.result_slug_got'),
        value: Intent.find(test.got['id']).slug
      }
    end
    if test.expected['solution'] == test.expected['got']
      diff_rows << {
        label: t('views.agent_regression_checks.result_solution_all'),
        value: JSON.parse(test.expected['solution'])
      }
    else
      diff_rows << {
        label: t('views.agent_regression_checks.result_solution_expected'),
        value: JSON.parse(test.expected['solution'])
      }
      diff_rows << {
        label: t('views.agent_regression_checks.result_solution_got'),
        value: JSON.parse(test.got['solution'])
      }
    end
  else
    diff_rows << {
      label: t('views.agent_regression_checks.result_slug_expected'),
      value: Intent.find(test.expected['id']).slug
    }
    diff_rows << {
      label: t('views.agent_regression_checks.result_solution_expected'),
      value: JSON.parse(test.expected['solution'])
    }
    diff_rows << {
      label: "Got",
      value: { message: t("views.agent_regression_checks.got_message.#{test.state}") }
    }
  end
  json.diff_rows diff_rows
end
