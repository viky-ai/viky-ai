$ = require('jquery');

class IntentList
  constructor: ->
    @sortable = Sortable.create($('#intents-list')[0], {
      handle: '.intents-list__draggable'
      animation: 100
      onUpdate: ->
        ids = []
        ids.push($(item).data('id')) for item in $('#intents-list > li')
        $.ajax
          url: $('#intents-list').data('update-positions-path')
          method: 'POST'
          data: { ids: ids }
    });

Setup = ->
  if $('body').data('controller-name') == "agents" && $('body').data('controller-action') == "show"
    new IntentList()

$(document).on('turbolinks:load', Setup)
