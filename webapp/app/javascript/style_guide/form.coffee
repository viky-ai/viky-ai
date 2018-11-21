$ = require('jquery');
autosize = require('autosize');

class StyleGuideForm
  constructor: ->
    autosize(textarea) for textarea in $('textarea.autosize')

Setup = ->
  new StyleGuideForm()

$(document).on('turbolinks:load', Setup)
