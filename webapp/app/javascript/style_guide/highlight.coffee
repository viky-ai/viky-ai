$ = require('jquery');

Setup = ->
  if $('body').data('controller-name') == "style_guide"
    Prism.highlightAll()

$(document).on('turbolinks:load', Setup)
