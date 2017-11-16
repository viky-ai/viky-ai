$ = require('jquery');

class InterpretationsList
  constructor: ->
    @sortable = Sortable.create($('#interpretations-list')[0], {
      handle: '.interpretations-list__draggable'
      animation: 100
      onUpdate: ->
        ids = []
        ids.push($(item).data('id')) for item in $('#interpretations-list > li')
        $.ajax
          url: $('#interpretations-list').data('update-positions-path')
          method: 'POST'
          data: { ids: ids }
    });

Setup = ->
  if $('body').data('controller-name') == "intents" && $('body').data('controller-action') == "show"
    new InterpretationsList()

$(document).on('turbolinks:load', Setup)
