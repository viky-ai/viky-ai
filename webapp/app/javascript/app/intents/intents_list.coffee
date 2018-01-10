$ = require('jquery');

class IntentList
  constructor:(visibility) ->
    list_selector_id = '#intents-list-' + visibility
    if $(list_selector_id).length == 1
      @sortable = Sortable.create($(list_selector_id)[0], {
        handle: '.intents-list__item__draggable'
        animation: 100
        onUpdate: ->
          ids = []
          ids.push($(item).data('id')) for item in $(list_selector_id + ' > li')
          $.ajax
            url: $(list_selector_id).data('update-positions-path')
            method: 'POST'
            data: { ids: ids }
      });

Setup = ->
  if $('body').data('controller-name') == "agents" && $('body').data('controller-action') == "show"
    new IntentList('is_public')
    new IntentList('is_private')

$(document).on('turbolinks:load', Setup)
