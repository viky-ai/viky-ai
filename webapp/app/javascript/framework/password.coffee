class PasswordInput
  constructor: ->
    @build_markup()
    @setup_listeners()

  build_markup: ->
    for input in $("input[type='password']")
      html = []

      $(input).replaceWith(
        $("<div class='password-show-hide'>").append($(input).clone())
          .append("<a href='#' class='password-show-hide__show'>Show</a>")
          .append("<a href='#' class='password-show-hide__hide' style='display:none;'>Hide</a>")
      )

  setup_listeners: ->
    for show in $('.password-show-hide__show')
      $(show).on 'click', (event) ->
        event.preventDefault()
        show_link = $(this)
        hide_link = $(this).closest('.password-show-hide').find('.password-show-hide__hide')
        input = $(this).closest('.password-show-hide').find("input[type='password']")
        show_link.hide()
        hide_link.show()
        input.attr('type', 'text')

    for hide in $('.password-show-hide__hide')
      $(hide).on 'click', (event) ->
        event.preventDefault()
        hide_link = $(this)
        show_link = $(this).closest('.password-show-hide').find('.password-show-hide__show')
        input = $(this).closest('.password-show-hide').find("input[type='text']")
        hide_link.hide()
        show_link.show()
        input.attr('type', 'password')

Setup = ->
  new PasswordInput()

$(document).on('turbolinks:load', Setup)
