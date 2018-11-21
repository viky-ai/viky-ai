$ = require('jquery')

class HighlightAndScrollTo
  constructor: ->
    intent_id = $(location).attr('hash').replace('smooth-scroll-to-', '')
    if intent_id
      if $(intent_id).length > 0
        $(intent_id).addClass('highlight')
        $(intent_id)[0].scrollIntoView({ block: 'center', behavior: 'smooth' })

Setup = ->
  new HighlightAndScrollTo()

$(document).on('turbolinks:load', Setup)
