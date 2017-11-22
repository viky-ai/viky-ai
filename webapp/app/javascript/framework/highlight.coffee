$ = require('jquery');

Setup = ->
  Prism.highlightAll()
  $("body").on 'code:highlight', (event) =>
    Prism.highlightAll()

$(document).on('turbolinks:load', Setup)
