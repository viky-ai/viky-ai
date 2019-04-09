$ = require('jquery');

class EntitiesImport
  constructor: ->
    $("body").on "entities_import:start",   (event, data) => @start(event, data)
    $("body").on "entities_import:failure", (event, data) => @failure(event, data)
    $("body").on "entities_import:success", (event, data) => @success(event, data)
    EntitiesImport.disableEdition() if EntitiesImport.running()

  start: (event, data) ->
    if data.entities_list_id == $('body').data('entities-list-id')
      $('#import-card').html(data.html)
      $('#import-card').show()
      EntitiesImport.disableEdition()

  success: (event, data) ->
    if data.entities_list_id == $('body').data('entities-list-id')
      $('#import-card').html(data.html)
      EntitiesImport.enableEdition()

      refreshCounter = setInterval ->
        counter = parseInt($('#import-card span.counter').html(), 10)
        $('#import-card span.counter').html(counter - 1)
      , 1000

      setTimeout ->
        Turbolinks.visit window.location.href
        clearInterval(refreshCounter);
      , 5000

  failure: (event, data) ->
    if data.entities_list_id == $('body').data('entities-list-id')
      if data.current_user_id == $('body').data('current-user')
        $('#import-card').html(data.html)
        EntitiesImport.enableEdition()
      else
        $('#import-card').html("").hide()
        EntitiesImport.enableEdition()

  @running: -> $('main .card--warning:visible').length == 1

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
