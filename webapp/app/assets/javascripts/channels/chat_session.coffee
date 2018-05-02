App.chat_session = App.cable.subscriptions.create "ChatSessionChannel",
  received: (data) ->
    if $(".chatbot").data('session-id') == data['session_id']

      if data["action"] == "display_message"
        new App.Statement(data['message']).display()

      if data["action"] == "reset"
        if data["path"].split('?')[0] == window.location.pathname.split('?')[0]
          Turbolinks.visit(data["path"])
