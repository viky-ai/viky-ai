$ = require('jquery');

class Nav
  constructor: ->
    if $('.h-nav').length == 1
      @update()
      $(window).bind 'load resize orientationchange', =>
        @update()

  update: ->
    $('nav .secondary').show()
    $('nav .more').hide()
    if $('nav > div').last().offset().top > $('nav > div').first().offset().top
      $('nav .secondary').hide()
      $('nav .more').show()

SetupNav = ->
  new Nav()

$(document).on('turbolinks:load', SetupNav)

