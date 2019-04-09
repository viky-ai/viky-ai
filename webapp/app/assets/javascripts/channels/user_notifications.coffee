App.cable.subscriptions.create "UserNotificationsChannel",
  connected: ->
    # Called when the subscription is ready for use on the server

  disconnected: ->
    # Called when the subscription has been terminated by the server

  received: (data) ->
    App.AgentDuplicator.end()

    if data.alert
      App.Message.alert(data.alert)

    if data.notice
      App.Message.notice(data.notice)
