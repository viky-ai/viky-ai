$ = require('jquery')

class IntentHighlight
  constructor: ->
    intent_id = $(location).attr('hash')
    if intent_id?
      App.CardListHelper.showHighlightedItem(intent_id)

Setup = ->
  if $('body').data('controller-name') == "intents" && $('body').data('controller-action') == "index"
    new IntentHighlight()

$(document).on('turbolinks:load', Setup)
