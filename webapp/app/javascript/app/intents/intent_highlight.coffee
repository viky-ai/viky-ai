$ = require('jquery')

class IntentHighlight
  constructor: ->
    intent_id = $(location).attr('hash')
    if intent_id?
      @showHighlightedIntent(intent_id)

  showHighlightedIntent: (id) ->
    element = $(id)
    if element?
      element.addClass('highlight')
      setTimeout ->
        element.removeClass('highlight')
      ,1000


Setup = ->
  if $('body').data('controller-name') == "intents" && $('body').data('controller-action') == "index"
    new IntentHighlight()

$(document).on('turbolinks:load', Setup)
