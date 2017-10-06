$ = require('jquery');

class CardShake
  constructor: ->
    $(".shake-card-trigger").on 'click', ->
      $(this).closest('.card').addClass('card--shake')
      init = =>
        $(this).closest('.card').removeClass('card--shake')
      setTimeout(init, 700)

Setup = ->
  if $('body').data('controller-name') == "style_guide"
    new CardShake()

$(document).on('turbolinks:load', Setup)
