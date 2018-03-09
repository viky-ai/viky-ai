$ = require('jquery');

class AgentSearchForm
  constructor: ->
    @setup()

  setup: ->
    App.FocusInput.atEnd('#search_query')

Setup = ->
  if $('body').data('controller-name') == "agents" && $('body').data('controller-action') == "index"
    new AgentSearchForm()

$(document).on('turbolinks:load', Setup)
