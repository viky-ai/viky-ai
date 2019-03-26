$ = require('jquery');

class EntitiesImportForm
  constructor: ->
    $("body").on "ajax:before", (event) =>
      @displayWaitMessage(event) if $(event.target).hasClass('js-import-entities-form')

    $("body").on "ajax:error", (event) =>
      @displayErrorMessage(event) if $(event.target).hasClass('js-import-entities-form')
    
    $("body").on "ajax:success", (event) =>
      @displaySuccessMessage(event) if $(event.target).hasClass('js-import-entities-form')

  displayWaitMessage: (event) ->
    @getModalMain().addClass('modal__main--loading')
    @getFormAndTitle().hide();
    @getWaitMessage().show();

  displayErrorMessage: (event) ->
    @getModalMain().removeClass('modal__main--loading')
    @getFormAndTitle().show();
    @getWaitMessage().hide();
  
  displaySuccessMessage: (event) ->
    @getWaitMessage().hide()
    $('#success-message').show()
    setTimeout(->
      $('#close-import').click()
    , 1000)
    

  getFormAndTitle: ->
    return $('#import-entities-form, .modal__main__title')

  getWaitMessage: ->
    return $('#wait-message')

  getModalMain: ->
    return $('.modal__main')

Setup = ->
  if $('body').data('controller-name') == "entities_lists" && $('body').data('controller-action') == "show"
    new EntitiesImportForm()

$(document).on('turbolinks:load', Setup)
