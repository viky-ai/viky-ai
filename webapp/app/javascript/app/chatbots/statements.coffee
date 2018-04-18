$ = require('jquery');

class Statement
  constructor: (html) ->
    @html = html

  is_from_user: ->
    $(@html).hasClass('chatbot__statement--user')

  is_from_bot: ->
    $(@html).hasClass('chatbot__statement--user')

  display: ->
    content = $(@html)
    content.find('.chatbot__avatar').addClass('chatbot__avatar--hidden')
    content.find('.chatbot__widget').addClass('chatbot__widget--hidden')
    $('.chatbot__discussion').append(content)

    avatar = $('.chatbot__discussion .chatbot__avatar').last()
    widget = $('.chatbot__discussion .chatbot__widget').last()
    @scroll_to_last()
    avatar.removeClass('chatbot__avatar--hidden')
    widget.removeClass('chatbot__widget--hidden')


  scroll_to_last: ->
    $(".chatbot__discussion").animate(
      { scrollTop: $('.chatbot__discussion').prop("scrollHeight")}, 250
    )

module.exports = Statement

