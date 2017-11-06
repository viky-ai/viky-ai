$ = require('jquery');

class StyleGuideMenu
  constructor: ->
    @build_nav()
    @build_numbers()

  build_numbers: ->
    i = 1
    for title in $('main section > h2')
      $(title).attr(id: "section-#{i}")
              .html("<small>#{i}.</small> #{$(title).html()} <a href='#section-#{i}' data-turbolinks='false'>#</a>")
      i++

    i = 1
    for section in $('main section')
      j = 1
      for h3 in $(section).find(" > h3")
        $(h3).attr(id: "section-#{i}-#{j}")
             .html("<small>#{i}.#{j}.</small> #{$(h3).html()} <a href='#section-#{i}-#{j}' data-turbolinks='false'>#</a>")
        j++
      i++

  build_nav: ->
    menu = []
    i = 1
    for title in $('main section > h2')
      menu.push "<li>"
      menu.push "  <a href='#section-#{i}'>"
      menu.push "    <small>#{i}.</small> #{$(title).html()}"
      menu.push "  </a>"
      menu.push "</li>"
      i++
    $('nav ul.style-guide').html(menu.join(''))


Setup = ->
  new StyleGuideMenu()

$(document).on('turbolinks:load', Setup)
