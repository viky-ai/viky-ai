App.chat_session = App.cable.subscriptions.create "ChatSessionChannel",
  received: (data) ->
    if $(".chatbot").data('session-id') == data['session_id']
      $(".chatbot__discussion").append(data['message'])
      $(".chatbot__discussion").animate(
        { scrollTop: $('.chatbot__discussion').prop("scrollHeight")}, 500
      )
