App.agent_regression_checks = App.cable.subscriptions.create 'AgentRegressionChecksChannel',

  received: (data) ->
    if $('.agent-header').data()['agent_id'] == data.agent_id
      if data.check_id == 'summary'
        if data.timestamp > App.ConsoleTestSuite.summary.timestamp
          App.ConsoleTestSuite.summary.status = data.state
          App.ConsoleTestSuite.summary.last_update = data.last_update
          App.ConsoleTestSuite.summary.failed_count_i18n = data.failed_count_i18n
          App.ConsoleTestSuite.summary.timestamp = data.timestamp
      else
        for test in App.ConsoleTestSuite.tests
          if data.check_id == test.id && data.timestamp > test.timestamp
            test.state = data.state
            test.timestamp = data.timestamp
