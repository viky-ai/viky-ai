$ = require('jquery');

class CardList
  constructor: (id, group_name) ->
    CardListHelper.updateDraggable()
    @list_selector_id = '#' + id
    @update_url = $(@list_selector_id).data('update-positions-path')
    @group_name = group_name
    if $(@list_selector_id).length == 1
      @sortable = Sortable.create($(@list_selector_id)[0], {
        group: @group_name
        handle: '.card-list__item__draggable'
        animation: 100,
        onAdd: =>
          if $(@list_selector_id + ' > li').children().length > 0
            $(@list_selector_id).removeClass('card-list--empty')
          if @update_url
            @updatePositions(@group_name)
        onRemove: =>
          if $(@list_selector_id + ' > li').children().length == 0
            $(@list_selector_id).addClass('card-list--empty')
        onUpdate: =>
          if @update_url
            @updatePositions(@group_name)
        onMove: (evt) ->
          for list in $(".card-list")
            if $(evt.related).id != list.id
              $(".card-list").removeClass('card-list--empty--dragging')
          if $(evt.related).is('ul')
            $(evt.related).addClass('card-list--empty--dragging')
          $('.tabs li.js-draggable-locale').removeClass('drag-in')
          if $(evt.to).hasClass('js-draggable-locale')
            $(evt.dragged).hide()
            $(evt.to).addClass('drag-in')
          else
            $(evt.dragged).show()
      })

  updatePositions: (group_name) ->
    if group_name != undefined
      group = $('ul.card-list').filter((index, element) -> $(element).data('group') == group_name)
    else
      group = $('ul.card-list')
    data = {}
    $(group).each((index, card_lists) ->
      $(card_lists).children('li').each((index, card_list) ->
        if data[$(card_lists).data('key')] == undefined
          data[$(card_lists).data('key')] = [$(card_list).data('id')]
        else
          data[$(card_lists).data('key')].push($(card_list).data('id'))
      )
    )
    $.ajax
      url: @update_url
      method: 'POST'
      data: data


class CardListHelper
  @addToList: (raw_html) ->
    html = $(raw_html).addClass('hidden')
    $('ul.card-list').prepend(html)
    setTimeout((() -> $(".card-list li").first().removeClass('hidden')), 0)

  @removeFromList: (id_selector) ->
    $(id_selector).addClass('deleted');
    setTimeout((() -> $(id_selector).remove()), 200)

  @updateItem: (id_selector, form) ->
    $(id_selector).html(form);

  @showItemDetails: (id_selector, show) ->
    $(id_selector).html(show);

  @updateBlankslates: ->
    if $(".card-list > li").length == 0
      if $(".card .tabs > ul > li").length <= 2
        $("#blankslate-start").show()
      else
        $("#blankslate-start-or-remove").show()
    else
      $("#blankslate-start").hide()
      $("#blankslate-start-or-remove").hide()

  @updateDraggable: ->
    if $('.tabs > ul > li.js-draggable-locale').length > 0
      $('.card-list li .card-list__item__draggable').show()
    else
      if $('.card-list li').length > 1
        $('.card-list li .card-list__item__draggable').show()
      else
        $('.card-list li .card-list__item__draggable').hide()

Setup = ->
  $('ul.card-list[data-behavior="sortable"]').each((index, element) ->
    new CardList($(element).attr('id'), $(element).data('group'))
  )

$(document).on('turbolinks:load', Setup)

module.exports = CardListHelper
