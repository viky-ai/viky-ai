$ = require('jquery');

class EntitiesImport
  constructor: ->
    $(document).on "turbolinks:before-visit", =>
      $(document).off "turbolinks:before-visit"
      clearInterval @progression    if @progression
      clearInterval @refreshCounter if @refreshCounter

    $("body").on "entities_import:start",   (event, data) => @start(event, data)
    $("body").on "entities_import:failure", (event, data) => @failure(event, data)
    $("body").on "entities_import:success", (event, data) => @success(event, data)
    if EntitiesImport.running()
      EntitiesImport.disableEdition()
      @update_progression()

  update_progression: ->
    progress = 0
    @progression = setInterval ->
      now = Date.now() / 1000
      duration = $("#progress").data('duration')
      start = $("#progress").data('start')
      if progress > 99
        progress = progress + 0.01
      else if progress > 98
        progress = progress + 0.05
      else if progress > 95
        progress = progress + 0.1
      else
        progress =  (now - start) * 100 / duration
      $("#progress .banner__progression__bar").css(width: "#{progress}%")
    , 750

  start: (event, data) ->
    if data.entities_list_id == $('body').data('entities-list-id')
      $('#import-card').html(data.html)
      $('#import-card').show()
      @update_progression()
      EntitiesImport.disableEdition()

  success: (event, data) ->
    if data.entities_list_id == $('body').data('entities-list-id')
      clearInterval @progression
      $('#import-card').css('min-height', $('#import-card').height());
      $('#import-card .banner').fadeTo(500, 0.25, =>
        $('#import-card').html(data.html)
        $('#import-card .banner').css('opacity', 0.25).fadeTo(500, 1)
        EntitiesImport.enableEdition()
        @refreshCounter = setInterval ->
          counter = parseInt($('#import-card span.counter').html(), 10)
          $('#import-card span.counter').html(counter - 1)
        , 1000
        setTimeout =>
          clearInterval(@refreshCounter);
          Turbolinks.visit window.location.href
        , 5000
      )

  failure: (event, data) ->
    if data.entities_list_id == $('body').data('entities-list-id')
      clearInterval @progression
      if data.import_user_id == $('body').data('current-user')
        $('#import-card').html(data.html)
        EntitiesImport.enableEdition()
      else
        $('#import-card').html("").hide()
        EntitiesImport.enableEdition()

  @running: -> $('main .js-import-running:visible').length == 1

  @disableEdition: ->
    # Import modal link
    $('#open-import-modal').hide()
    # Forms
    $('.entity-form').prop("disabled", true)
    $('.entity-form input, .entity-form textarea').prop("disabled", true)
    $('.entity-form .field--submit button').prop("disabled", true).addClass('btn--disabled')
    $('.entity-form a.btn--destructive').prop("disabled", true).addClass('btn--disabled')
    for editor in $('.entity-form .CodeMirror-wrap')
      editor.CodeMirror.setOption('readOnly', "nocursor");
      $(editor).addClass('readonly');

  @enableEdition: ->
    # Import modal link
    $('#open-import-modal').show()
    # Forms
    $('.entity-form').prop("disabled", false)
    $('.entity-form input, .entity-form textarea').prop("disabled", false)
    $('.entity-form .field--submit button').prop("disabled", false).removeClass('btn--disabled')
    $('.entity-form a.btn--destructive').prop("disabled", false).removeClass('btn--disabled')
    for editor in $('.entity-form .CodeMirror-wrap')
      editor.CodeMirror.setOption('readOnly', false);
      $(editor).removeClass('readonly');

Setup = ->
  if $('body').data('controller-name') == 'entities_lists' && $('body').data('controller-action') == 'show'
    new EntitiesImport()

$(document).on('turbolinks:load', Setup)

module.exports = EntitiesImport
