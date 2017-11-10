$ = require('jquery');

class Console
  constructor: ->
    $("body").on 'click', (event) => @dispatch(event)

  dispatch: (event) ->
    link = $(event.target)
    if link.data('action') == 'console-switch-to-explain'
      event.preventDefault()
      link.closest('ul').find('a').removeClass('current')
      link.addClass('current')
      $("#console-explain").show()
      $("#console-json").hide()

    if link.data('action') == 'console-switch-to-json'
      event.preventDefault()
      link.closest('ul').find('a').removeClass('current')
      link.addClass('current')
      $("#console-explain").hide()
      $("#console-json").show()

Setup = ->
  new Console()

$(document).on('turbolinks:load', Setup)
