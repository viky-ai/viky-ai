App.agent_regression_checks = App.cable.subscriptions.create 'AgentRegressionChecksChannel',

  received: (data) ->
    if $('.agent-header').data()['agent_id'] == data.agent_id
      console.log(data)
