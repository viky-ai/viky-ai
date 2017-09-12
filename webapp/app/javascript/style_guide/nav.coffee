$ = require('jQuery');

class StyleGuideMenu
  constructor: ->
    menu = []
    i = 0
    for title in $('h2')
      $(title).attr(id: "section-#{i}")
      menu.push "<li>"
      menu.push "  <a href='#section-#{i}'>"
      menu.push "    <small>#{i}.</small>"
      menu.push "    #{$(title).html()}"
      menu.push "  </a>"
      menu.push "</li>"
      i++
    $('nav ul').html(menu.join(''))

Setup = ->
  if $('body').data('controller-name') == "style_guide"
    new StyleGuideMenu()

$(document).on('turbolinks:load', Setup)
