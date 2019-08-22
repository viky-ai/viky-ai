class PlayForm
  constructor: ->
    console.log "Hello Play UI"

Setup = ->
  if $('body').data('controller-name') == "play"
    new PlayForm()

$(document).on('turbolinks:load', Setup)
