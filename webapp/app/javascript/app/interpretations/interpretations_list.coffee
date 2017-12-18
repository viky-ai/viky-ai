$ = require('jquery');

class InterpretationsList
  constructor: ->
    InterpretationsList.updateDraggable()

    @sortable = Sortable.create($('#interpretations-list')[0], {
      group: "interpretations",
      handle: '.interpretations-list__draggable'
      animation: 100
      onUpdate: ->
        ids = []
        ids.push($(item).data('id')) for item in $('#interpretations-list > li')
        $.ajax
          url: $('#interpretations-list').data('update-positions-path')
          method: 'POST'
          data: { ids: ids }

      onMove: (event) ->
        $('.tabs li.js-draggable-locale').removeClass('drag-in')
        if $(event.to).hasClass('js-draggable-locale')
          $(event.dragged).hide()
          $(event.to).addClass('drag-in')
        else
          $(event.dragged).show()
    });

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

    InterpretationsList.displayCodeEditor($("#solution__new_interpretation")[0])

  @updateDraggable: ->
    if $('.tabs > ul > li.js-draggable-locale').length > 0
      $('#interpretations-list li .interpretations-list__draggable').show()
    else
      if $('#interpretations-list li').length > 1
        $('#interpretations-list li .interpretations-list__draggable').show()
      else
        $('#interpretations-list li .interpretations-list__draggable').hide()

  @updateBlankslates: ->
    if $("#interpretations-list > li").length == 0
      if $(".interpretation-new-form-container .tabs > ul > li").length == 2
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

  @displayCodeEditor: (textarea)->
    CodeMirror.fromTextArea(textarea, {
      lineNumbers: true,
      mode: "javascript",
      autoRefresh: true,
      tabSize: 2,
      insertSoftTab: true
    });

module.exports = InterpretationsList

Setup = ->
  if $('body').data('controller-name') == "intents" && $('body').data('controller-action') == "show"
    new InterpretationsList()

$(document).on('turbolinks:load', Setup)
