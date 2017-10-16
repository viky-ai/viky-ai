$ = require('jquery');

class Message
  constructor: ->

    if $('.message').length == 1
      $('.message').show().addClass('message--show')
      $("body").on 'click', (event) => @dispatch(event)
      setTimeout(=>
        @hide()
      , 10000)

  dispatch: (event) ->
    node   = $(event.target)
    action = node.data('action')
    if not action?
      node = $(event.target).closest('a')
      action = node.data('action')

    if action is "message-close"
      event.preventDefault()
      @hide()

  hide: ->
    $('.message').removeClass('message--show').addClass('message--hide')
    setTimeout(->
      $('.message').removeClass('message--hide').hide()
    , 250)


Setup = ->
  new Message()

$(document).on('turbolinks:load', Setup)
