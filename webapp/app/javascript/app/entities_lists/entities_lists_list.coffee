$ = require('jquery');

class EntitiesListList
  constructor:(visibility) ->
    list_selector_id= '#entities_lists-list-' + visibility
    if $(list_selector_id).length == 1
      @sortable = Sortable.create($(list_selector_id)[0], {
        group: 'entities_list'
        handle: '.card-list__item__draggable'
        animation: 100,
        onAdd: ->
          if $(list_selector_id+ ' > li').children().length > 0
            $(list_selector_id).removeClass('card-list--empty')
          ids = []
          ids.push($(item).data('id')) for item in $(list_selector_id+ ' > li')
          EntitiesListList::updatePositions(list_selector_id)
        onRemove: ->
          if $(list_selector_id+ ' > li').children().length == 0
            $(list_selector_id).addClass('card-list--empty')
          ids = []
          ids.push($(item).data('id')) for item in $(list_selector_id+ ' > li')
        onUpdate: ->
          ids = []
          ids.push($(item).data('id')) for item in $(list_selector_id+ ' > li')
          EntitiesListList::updatePositions(list_selector_id, ids, visibility)
        onMove: (evt) ->
          for list in $(".card-list")
            if $(evt.related).id != list.id
              $(".card-list").removeClass('card-list--empty--dragging')
          if $(evt.related).is('ul')
            $(evt.related).addClass('card-list--empty--dragging')
      });

  updatePositions:(list_selector_id) ->
    is_public = []
    is_public.push($(item).data('id')) for item in $('#entities_lists-list-is_public' + ' > li')
    is_private = []
    is_private.push($(item).data('id')) for item in $('#entities_lists-list-is_private' + ' > li')
    $.ajax
      url: $(list_selector_id).data('update-positions-path')
      method: 'POST'
      data: { is_public: is_public, is_private: is_private }


Setup = ->
  if $('body').data('controller-name') == "agents" && $('body').data('controller-action') == "show"
    new EntitiesListList('is_public')
    new EntitiesListList('is_private')

$(document).on('turbolinks:load', Setup)
