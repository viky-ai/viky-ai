$ = require('jquery');

class BotForm
  constructor: ->
    $('body').on 'modal:load', (event) =>
      @setup() if $("#modal_container .bot-form").length == 1

    $("body").on 'click', (event) => @dispatch(event)


  dispatch: (event) ->
    link = $(event.target)
    action = link.data('action')

    if action == 'bot-ping-endpoint'
      event.preventDefault()

      $.ajax
        url: link.data('url')
        method: 'GET'
        data: { endpoint: $('#bot_endpoint').val() }
        beforeSend: ->
          link.html(link.data('loading'))

        success: (data) ->
          succeed = data['succeed']
          message = data['message']
          link.html(link.data('idle'))
          if succeed
            $('#ping-result').addClass('help--success').removeClass('help--error')
          else
            $('#ping-result').removeClass('help--success').addClass('help--error')

          $('#ping-result').show()
          $('#ping-result-content').html(message)


  setup: ->
    if $('.field_with_errors input').length == 0
      App.FocusInput.atEnd('#bot_name')
    else
      App.FocusInput.atEnd('.field_with_errors input')



Setup = ->
  if $('body').data('controller-name') == "bots"
    new BotForm()

$(document).on('turbolinks:load', Setup)
