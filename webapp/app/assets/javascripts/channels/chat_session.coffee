document.addEventListener "turbolinks:load", ->

  if $('body').data('controller-name') == 'chatbots' && $('body').data('controller-action') == 'show'
    unless App.chat_session
      App.chat_session = App.cable.subscriptions.create "ChatSessionChannel",

        received: (data) ->
          if $(".chatbot").data('session-id') == data['session_id']

            if data["action"] == "display_message"
              new App.Statement(data['message']).display()

            if data["action"] == "reset"
              if data["path"].split('?')[0] == window.location.pathname.split('?')[0]
                Turbolinks.visit(data["path"])

            if data["action"] == "update_locale"
              $('.chatbot').first().data('recognition-locale', data["locale"])
              $('body').trigger 'recognition:update_locale'

  else
    if App.chat_session
      App.chat_session.unsubscribe()
      App.chat_session = null
