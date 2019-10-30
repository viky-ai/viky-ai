class Nav
  constructor: ->
    if $('.h-nav').length == 1
      @logo = $('.h-nav__header')
      @user = $($('.h-nav__content')[1])

      @update()
      $(window).bind 'load resize orientationchange', => @update()
      $("body").on "console:leave-fullscreen", => @update()

  update: ->
    @update_user()
    @update_logo()

  update_user: ->
    @user.removeClass('h-nav__content--hide-user-data')
    if @user.offset().top > 0
      @user.addClass('h-nav__content--hide-user-data')

  update_logo: ->
    @logo.removeClass('h-nav__header--hidden')
    if @user.offset().top > 0
      @logo.addClass('h-nav__header--hidden')

Setup = -> new Nav()

$(document).on('turbolinks:load', Setup)
