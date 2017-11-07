$ = require('jquery');

class IntentForm
  constructor: ->
    $('body').on 'modal:load', (event) =>
      @setup() if $("#modal_container .js-intent-form").length == 1

  setup: ->
    if $('.field_with_errors input').length == 0
      $('#intent_intentname').focus()
    else
      $('.field_with_errors input').first().focus()

Setup = ->
  if $('body').data('controller-name') == "agents"
    new IntentForm()

$(document).on('turbolinks:load', Setup)
