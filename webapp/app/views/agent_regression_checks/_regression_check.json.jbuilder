json.id        test.id
json.state     test.state
json.sentence  test.sentence
json.language  test.language
json.position  test.position

json.state_i18n  t("views.agent_regression_checks.indicator.#{test.state}")
json.now         test.now
json.update_url  test.id.nil? ? '' : user_agent_agent_regression_check_path(owner, agent, test)
json.delete_url  test.id.nil? ? '' : user_agent_agent_regression_check_path(owner, agent, test)
json.create_url  user_agent_agent_regression_checks_path(owner, agent)

if test.expected.present?
  diff_rows = []
  intent = Intent.find_by_id(test.expected['id'])
  if intent.nil?
    diff_rows << {
      error: t("views.agent_regression_checks.result.error_no_intent_html", icon: icon_close)
    }
  else
    if test.success?
      diff_rows << {
        label: t('views.agent_regression_checks.result.slug.expected_got_ok_html', icon: icon_check),
        value: intent.slug
      }
      diff_rows << {
        label: t('views.agent_regression_checks.result.solution.expected_got_ok_html', icon: icon_check),
        value: JSON.parse(test.expected['solution'])
      }
    elsif test.failure?
      if test.got.present?
        if test.expected['id'] == test.got['id']
          diff_rows << {
            label: t('views.agent_regression_checks.result.slug.expected_got_ok_html', icon: icon_check),
            value: intent.slug
          }
        else
          diff_rows << {
            label: t('views.agent_regression_checks.result.slug.expected_got_ko_html', icon: icon_close),
            value: intent.slug
          }
          diff_rows << {
            value: intent.slug
          }
        end
        if test.expected['solution'] == test.expected['got']
          diff_rows << {
            label: t('views.agent_regression_checks.result.solution.expected_got_ok_html', icon: icon_check),
            value: JSON.parse(test.expected['solution'])
          }
        else
          diff_rows << {
            label: t('views.agent_regression_checks.result.solution.expected_got_ko_html', icon: icon_close),
            value: JSON.parse(test.expected['solution'])
          }
          diff_rows << {
            value: JSON.parse(test.got['solution'])
          }
        end
      else
        diff_rows << {
          label: t('views.agent_regression_checks.result.slug.expected_ko_html', icon: icon_close),
          value: intent.slug
        }
        diff_rows << {
          label: t('views.agent_regression_checks.result.solution.expected_ko_html', icon: icon_close),
          value: JSON.parse(test.expected['solution'])
        }
        diff_rows << {
          error: t("views.agent_regression_checks.result.error_no_intent_html", icon: icon_close)
        }
      end
    elsif test.running? || test.unknown?
      diff_rows << {
        label: t('views.agent_regression_checks.result.slug.expected'),
        value: intent.slug
      }
      diff_rows << {
        label: t('views.agent_regression_checks.result.solution.expected'),
        value: JSON.parse(test.expected['solution'])
      }
    else # state == :error
      diff_rows << {
        label: t('views.agent_regression_checks.result.slug.expected_ko_html', icon: icon_close),
        value: intent.slug
      }
      diff_rows << {
        label: t('views.agent_regression_checks.result.solution.expected_ko_html', icon: icon_close),
        value: JSON.parse(test.expected['solution'])
      }
      diff_rows << {
        error: t("views.agent_regression_checks.result.error_#{test.state}_html", icon: icon_close)
      }
    end
  end
  json.diff_rows diff_rows
end
