class AgentConsoleChannel
  @_current_instance: null

  @destroy: ->
    if @_current_instance
      @_current_instance.unsubscribe()
      @_current_instance = null

  constructor: (agent_id) ->
    @identifier = { channel: 'AgentConsoleChannel', agent_id: agent_id }

  setup: ->
    if @constructor._current_instance
      if JSON.stringify(@identifier) != @constructor._current_instance.identifier
        AgentConsoleChannel.destroy()
        @create()
    else
      @create()

  create: =>
    @constructor._current_instance = App.cable.subscriptions.create @identifier,
      received: (data) ->
        if data.timestamp > App.ConsoleTestSuite.timestamp
          App.ConsoleFooter.summary = data.payload.summary
          App.ConsoleTestSuite.summary = data.payload.summary
          App.ConsoleTestSuite.tests = data.payload.tests
          if App.ConsoleExplainFooter?
            for test in data.payload.tests
              if test.id == App.ConsoleExplainFooter.test.id
                App.ConsoleExplainFooter.test = test

Setup = ->
  if $('.console-container').length == 1
    agent_id = document.querySelector('main > .agent-header').dataset['agent_id']
    if $('.console-container').length == 1 && agent_id
      new AgentConsoleChannel(agent_id).setup()
    else
      AgentConsoleChannel.destroy()

document.addEventListener "turbolinks:load", Setup
