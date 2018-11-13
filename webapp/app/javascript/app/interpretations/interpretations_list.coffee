$ = require('jquery');

class InterpretationsList
  constructor: ->
    App.CardListHelper.updateDraggable()

    for li in $('.tabs > ul > li.js-draggable-locale')
      Sortable.create(li, {
        group: "interpretations",
        animation: 100
        onAdd: (event) ->
          $('.tabs li.js-draggable-locale').removeClass('drag-in')
          $(event.target).addClass('drag-done')
          $.ajax
            url: $(event.item).data('update-locale-path')
            method: 'POST'
            data: { locale: $(event.target).data('locale') }
      });
    App.CodeEditor.buildJavaScriptEditor($("#solution__new_interpretation")[0])

    interpretation_id = $(location).attr('hash')
    unless interpretation_id == ''
       App.CardListHelper.showHighlightedItem(interpretation_id)

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
