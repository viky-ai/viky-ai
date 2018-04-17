$ = require('jquery');

class BotForm
  constructor: ->
    $('body').on 'modal:load', (event) =>
      @setup() if $("#modal_container .bot-form").length == 1

  setup: ->
    if $('.field_with_errors input').length == 0
      App.FocusInput.atEnd('#bot_name')
    else
      App.FocusInput.atEnd('.field_with_errors input')

Setup = ->
  if $('body').data('controller-name') == "bots"
    new BotForm()

$(document).on('turbolinks:load', Setup)
