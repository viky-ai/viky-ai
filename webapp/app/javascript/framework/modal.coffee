$ = require('jquery');

class Modal
  constructor: ->
    $("body").on 'click', (event) => @dispatch(event)

  dispatch: (event) ->
    node   = $(event.target)
    action = node.data('action')

    if not action?
      node = $(event.target).closest('a')
      action = node.data('action')
    if not action?
      node = $(event.target).closest('button')
      action = node.data('action')

    if action is "open-modal"
      event.preventDefault()
      modal_selector = node.data('modal-selector')
      $(modal_selector).show()
      $(document).on 'keyup', (e) => @close() if e.keyCode == 27

    if action is "close-modal"
      event.preventDefault()
      @close()

  close: ->
    $('.modal').hide()
    $(document).off 'keyup'

Setup = ->
  new Modal()

$(document).on('turbolinks:load', Setup)
