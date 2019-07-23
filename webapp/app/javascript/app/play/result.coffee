window.Popper = require('popper.js').default;

class PlayResult
  constructor: ->
    $('.play-main__result .highlight-pop').hide()

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
          new Popper(item, popup)
          $('.play-main__result__wrapper .highlight-pop').hide()
          item.addClass('highlight--open')
          popup.show()
      else
        $('.play-main__result__wrapper .highlight-pop').hide()
        $('.play-main__result__wrapper .highlight').removeClass('highlight--open')

module.exports = PlayResult
