$ = require('jquery');

class EntitiesImportForm
  constructor: ->
    $("body").on "ajax:before", (event) =>
      @displayWaitMessage(event) if $(event.target).hasClass('js-import-entities-form')
    $("body").on "ajax:error", (event) =>
      @displayErrorMessage(event) if $(event.target).hasClass('js-import-entities-form')

  displayWaitMessage: (event) ->
    @findForm().css(display: 'none');
    @findWaitMessage().css(display: '');

  displayErrorMessage: (event) ->
    @findForm().css(display: '');
    @findWaitMessage().css(display: 'none');

  findForm: ->
    return $('#import-entities-form')

  findWaitMessage: ->
    return $('#wait-message')

Setup = ->
  if $('body').data('controller-name') == "entities_lists" && $('body').data('controller-action') == "show"
    new EntitiesImportForm()

$(document).on('turbolinks:load', Setup)
