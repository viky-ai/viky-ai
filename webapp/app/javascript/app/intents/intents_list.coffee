$ = require('jquery');

class IntentList
  constructor:(visibility) ->
    list_selector_id= '#intents-list-' + visibility
    if $(list_selector_id).length == 1
      @sortable = Sortable.create($(list_selector_id)[0], {
        group: 'intent-list'
        handle: '.intents-list__item__draggable'
        animation: 100,
        onAdd: ->
          if $(list_selector_id+ ' > li').children().length > 0
            $(list_selector_id).removeClass('intents-list--empty')
          ids = []
          ids.push($(item).data('id')) for item in $(list_selector_id+ ' > li')
          IntentList::updatePositions(list_selector_id, ids, visibility)
        onRemove: ->
          if $(list_selector_id+ ' > li').children().length == 0
            $(list_selector_id).addClass('intents-list--empty')
          ids = []
          ids.push($(item).data('id')) for item in $(list_selector_id+ ' > li')
          IntentList::updatePositions(list_selector_id, ids, visibility)
        onUpdate: ->
          ids = []
          ids.push($(item).data('id')) for item in $(list_selector_id+ ' > li')
          IntentList::updatePositions(list_selector_id, ids, visibility)
        onMove: (evt) ->
          $(".intents-list").removeClass('intents-list--empty--dragging')
          if $(evt.related).is('ul')
            $(evt.related).addClass('intents-list--empty--dragging')
      });

  updatePositions:(list_selector_id, ids, visibility) ->
    $.ajax
      url: $(list_selector_id).data('update-positions-path') + '?visibility=' + visibility
      method: 'POST'
      data: { ids: ids }

Setup = ->
  if $('body').data('controller-name') == "agents" && $('body').data('controller-action') == "show"
    new IntentList('is_public')
    new IntentList('is_private')

$(document).on('turbolinks:load', Setup)
