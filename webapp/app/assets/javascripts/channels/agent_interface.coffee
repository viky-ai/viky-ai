class AgentInterfaceChannel
  @_current_instance: null

  @destroy: ->
    if @_current_instance
      @_current_instance.unsubscribe()
      @_current_instance = null

  constructor: (agent_id) ->
    @identifier = { channel: 'AgentInterfaceChannel', agent_id: agent_id }

  setup: ->
    if @constructor._current_instance
      if JSON.stringify(@identifier) != @constructor._current_instance.identifier
        AgentInterfaceChannel.destroy()
        @create()
    else
      @create()

  create: =>
    @constructor._current_instance = App.cable.subscriptions.create @identifier,
      received: (data) ->
        if data.trigger
          $("body").trigger(data.trigger.event, data.trigger.data)

Setup = ->
  if $('.console-container').length == 1
    agent_id = $('body').data('agent-id')
    if $('.console-container').length == 1 && agent_id
      new AgentInterfaceChannel(agent_id).setup()
    else
      AgentInterfaceChannel.destroy()

document.addEventListener "turbolinks:load", Setup
