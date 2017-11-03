$ = require('jquery');

Setup = ->
  Prism.highlightAll()

$(document).on('turbolinks:load', Setup)
