$ = require('jquery');

class TransferOwnershipForm
  constructor: ->
    $('body').on 'modal:load', (event) =>
      @setup() if $("#modal_container .js-user-search").length == 1

  setup: ->
    if $('.field_with_errors input').length == 0
      App.FocusInput.atEnd('#input-user-search-selectized')
    else
      App.FocusInput.atEnd('.field_with_errors input')

Setup = ->
  if $('body').data('controller-name') == "agents"
    new TransferOwnershipForm()

$(document).on('turbolinks:load', Setup)
