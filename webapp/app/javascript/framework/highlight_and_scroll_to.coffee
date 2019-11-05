$ = require('jquery')

class HighlightAndScrollTo
  constructor: ->
    interpretation_id = $(location).attr('hash').replace('smooth-scroll-to-', '')
    if interpretation_id
      if $(interpretation_id).length > 0
        $(interpretation_id).addClass('highlight')
        $(interpretation_id)[0].scrollIntoView({ block: 'center', behavior: 'smooth' })

Setup = ->
  new HighlightAndScrollTo()

$(document).on('turbolinks:load', Setup)
