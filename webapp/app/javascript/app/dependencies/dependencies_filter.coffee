$ = require('jquery');

class DependenciesFilter
  constructor: ->
    $('body').on 'modal:load', (event) =>
      @setup() if $("#modal_container #new_search").length == 1

  setup: ->
    App.FocusInput.atEnd('#search_query')

Setup = ->
  if $('body').data('controller-name') == "agents"
    new DependenciesFilter()

$(document).on('turbolinks:load', Setup)
