$(document).on "turbolinks:load", ->
  if $('.console-container').length == 1
    agent_id = $('.agent-header').data()['agent_id']
    App.agent_regression_checks = App.cable.subscriptions.create { channel: 'AgentRegressionChecksChannel', agent_id: agent_id },
      received: (data) ->
        if data.timestamp > App.ConsoleTestSuite.timestamp
          App.ConsoleFooter.summary = data.payload.summary
          App.ConsoleTestSuite.summary = data.payload.summary
          App.ConsoleTestSuite.tests = data.payload.tests
          if App.ConsoleExplainFooter?
            for test in data.payload.tests
              if test.id == App.ConsoleExplainFooter.test.id
                App.ConsoleExplainFooter.test = test
  else
    if App.agent_regression_checks
      App.agent_regression_checks.unsubscribe()
      App.agent_regression_checks = null
