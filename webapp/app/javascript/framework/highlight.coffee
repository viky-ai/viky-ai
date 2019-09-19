Setup = ->
  for pre in $(".markdown pre")
    lang = $(pre).attr('lang')
    code = $(pre).find('code')
    code.addClass("language-#{lang}")

  Prism.highlightAll()
  $("body").on 'code:highlight', (event) =>
    Prism.highlightAll()

$(document).on('turbolinks:load', Setup)
