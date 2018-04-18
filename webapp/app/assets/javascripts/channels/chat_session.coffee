App.chat_session = App.cable.subscriptions.create "ChatSessionChannel",
  received: (data) ->
    if $(".chatbot").data('session-id') == data['session_id']
      new App.Statement(data['message']).display()
