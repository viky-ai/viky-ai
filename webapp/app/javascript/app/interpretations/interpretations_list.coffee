$ = require('jquery');

class InterpretationsList
  constructor: ->
    InterpretationsList.updateDraggable()
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

  @updateDraggable: ->
    if $('#interpretations-list li').length > 1
      $('#interpretations-list li .interpretations-list__draggable').show()
    else
      $('#interpretations-list li .interpretations-list__draggable').hide()

  @updateBlankslates: ->
    if $("#interpretations-list > li").length == 0
      if $(".tabs > ul > li").length == 2
        $("#blankslate-start").show()
      else
        $("#blankslate-start-or-remove").show()
    else
      $("#blankslate-start").hide()
      $("#blankslate-start-or-remove").hide()

  @incrementTabBadge: ->
    count = $("#current-locale-tab-badge").data('count')
    $("#current-locale-tab-badge").html(count + 1)
    $("#current-locale-tab-badge").data('count',  count + 1)

  @decrementTabBadge: ->
    count = $("#current-locale-tab-badge").data('count')
    $("#current-locale-tab-badge").html(count - 1)
    $("#current-locale-tab-badge").data('count',  count - 1)


module.exports = InterpretationsList

Setup = ->
  if $('body').data('controller-name') == "intents" && $('body').data('controller-action') == "show"
    new InterpretationsList()

$(document).on('turbolinks:load', Setup)
