App.agent_regression_checks = App.cable.subscriptions.create 'AgentRegressionChecksChannel',

  received: (data) ->
    if $('.agent-header').data()['agent_id'] == data.agent_id
      for test in App.ConsoleTestSuite.tests
        if data.check_id == test.id
          test.state = data.state

      if App.ConsoleTestSuite.tests.some((check) -> check.state == 'failure')
        App.ConsoleTestSuite.summary.status = 'failure'
      else
        App.ConsoleTestSuite.summary.status = 'success'
