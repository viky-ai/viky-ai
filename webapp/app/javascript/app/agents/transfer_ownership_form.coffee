class TransferOwnershipForm
  constructor: ->
    $('body').on 'modal:load', (event) => @setup()

  setup: ->
    if $("#modal_container .js-transfert-ownership-form").length == 1
      App.FocusInput.atEnd('#js-new-owner')

Setup = ->
  if $('body').data('controller-name') == "agents"
    new TransferOwnershipForm()

$(document).on('turbolinks:load', Setup)
