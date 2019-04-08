$ = require('jquery');

class EntitiesImport
  constructor: ->
    $("body").on "entities_import:success", (event, data) => @success(event, data)
    $("body").on "entities_import:failure", (event, data) => @failure(event, data)

  success: (event, data) ->
    $('#import-card').html(data.html)

  failure: (event, data) ->
    $('#import-card').html(data.html)

Setup = ->
  if $('body').data('controller-name') == 'entities_lists' && $('body').data('controller-action') == 'show'
    new EntitiesImport()

$(document).on('turbolinks:load', Setup)

module.exports = EntitiesImport
