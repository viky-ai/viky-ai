class FormulationsList
  constructor: ->
    App.CardListHelper.updateDraggable()

    for li in $('.tabs > ul > li.js-draggable-locale')
      Sortable.create(li, {
        group: "formulations",
        animation: 100
        onAdd: (event) ->
          $('.tabs li.js-draggable-locale').removeClass('drag-in')
          $(event.target).addClass('drag-done')
          $.ajax
            url: $(event.item).data('update-locale-path')
            method: 'POST'
            data: { locale: $(event.target).data('locale') }
      });
    App.CodeEditor.buildJavaScriptEditor($("#solution__new_formulation")[0])

  @incrementTabBadge: ->
    count = $("#current-locale-tab-badge").data('count')
    $("#current-locale-tab-badge").html(count + 1)
    $("#current-locale-tab-badge").data('count',  count + 1)

  @decrementTabBadge: ->
    count = $("#current-locale-tab-badge").data('count')
    $("#current-locale-tab-badge").html(count - 1)
    $("#current-locale-tab-badge").data('count',  count - 1)

module.exports = FormulationsList

Setup = ->
  if $('body').data('controller-name') == "intents" && $('body').data('controller-action') == "show"
    new FormulationsList()

$(document).on('turbolinks:load', Setup)
