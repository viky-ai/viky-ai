window.Popper = require('popper.js').default;

class PlayResult
  constructor: ->
    $('.play-main__result .highlight-pop').hide()

    $('.play-main__result__wrapper .highlight').on 'mouseenter', (event) ->
      item = $(event.target)
      popup = item.prev('.highlight-pop')
      pkg = popup.data('package')
      color = popup.data('color')

      $("#agent-compact-#{pkg}").addClass("agent-compact--highlight-#{color}")

    $('.play-main__result__wrapper .highlight').on 'mouseleave', (event) ->
      $('aside .agent-compact').removeClass().addClass('agent-compact')

    $('.play-main__result__wrapper').on 'click', (event) ->
      item = $(event.target)
      if item.hasClass('highlight')
        event.preventDefault()
        popup = item.prev('.highlight-pop')
        if popup.is(':visible')
          item.removeClass('highlight--open')
          popup.hide()
        else
          $('.play-main__result__wrapper .highlight').removeClass('highlight--open')
          new Popper(item, popup, { placement: 'auto' })
          $('.play-main__result__wrapper .highlight-pop').hide()
          item.addClass('highlight--open')
          popup.show()
      else
        if item.closest('.highlight-pop').length == 0
          $('.play-main__result__wrapper .highlight-pop').hide()
          $('.play-main__result__wrapper .highlight').removeClass('highlight--open')

Setup = ->
  if $('body').data('controller-name') == "play"
    new PlayResult()

$(document).on('turbolinks:load', Setup)

module.exports = PlayResult
