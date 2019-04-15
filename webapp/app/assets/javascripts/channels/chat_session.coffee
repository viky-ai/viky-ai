class ChatSessionChannel
  @_current_instance: null

  @destroy: ->
    if @_current_instance
      @_current_instance.unsubscribe()
      @_current_instance = null

  constructor: ->
    @identifier = "ChatSessionChannel"

  setup: ->
    unless @constructor._current_instance
      @create()

  create: =>
    @constructor._current_instance = App.cable.subscriptions.create @identifier,
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

Setup = ->
  if $('body').data('controller-name') == 'chatbots' && $('body').data('controller-action') == 'show'
    new ChatSessionChannel().setup()
  else
    ChatSessionChannel.destroy()

document.addEventListener "turbolinks:load", Setup
