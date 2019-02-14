App.agent_regression_checks = App.cable.subscriptions.create 'AgentRegressionChecksChannel',

  received: (data) ->
    if $('.agent-header').data()['agent_id'] == data.agent_id
      if data.timestamp > App.ConsoleTestSuite.timestamp
        App.ConsoleFooter.summary = data.payload.summary
        App.ConsoleTestSuite.summary = data.payload.summary
        App.ConsoleTestSuite.tests = data.payload.tests
        if App.ConsoleExplainFooter?
          for test in data.payload.tests
            if test.id == App.ConsoleExplainFooter.test.id
              App.ConsoleExplainFooter.test = test
