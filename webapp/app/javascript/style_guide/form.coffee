$ = require('jquery');
autosize = require('autosize');

class StyleGuideForm
  constructor: ->
    autosize(textarea) for textarea in $('textarea.autosize')
    console.log "StyleGuideForm"

Setup = ->
  new StyleGuideForm()

$(document).on('turbolinks:load', Setup)
