$ = require('jquery');

class Message
  TIMEOUT: null

  constructor: ->
    $("body").on 'click', (event) => @dispatch(event)
    Message.show() if $('.message').length == 1

  dispatch: (event) ->
    node   = $(event.target)
    action = node.data('action')
    if not action?
      node = $(event.target).closest('a')
      action = node.data('action')

    if action is "message-close"
      event.preventDefault()
      Message.hide()

  @notice: (message) ->
    @build(message, 'notice')
    @show()

  @alert: (message) ->
    @build(message, 'alert')
    @show()

  @show: ->
    clearTimeout(@TIMEOUT) if @TIMEOUT
    $('.message').show().addClass('message--show')
    @TIMEOUT = setTimeout ->
      Message.hide()
    , 5000

  @hide: ->
    $('.message').removeClass('message--show').addClass('message--hide')
    setTimeout(->
      $('.message').remove()
    , 225)

  @build: (message, type) ->
    $('.message').remove()
    html = []
    html.push "<div class='message message--#{type}'>"
    html.push "  <div class='message__content'>"
    html.push message
    html.push "  </div>"
    html.push "  <div class='message__action'>"
    html.push '    <a href="#" data-action="message-close"><span class="icon"><svg xmlns="http://www.w3.org/2000/svg" xmlns:xlink="http://www.w3.org/1999/xlink" version="1.1" width="24" height="24" viewBox="0 0 24 24"><path d="M19,6.41L17.59,5L12,10.59L6.41,5L5,6.41L10.59,12L5,17.59L6.41,19L12,13.41L17.59,19L19,17.59L13.41,12L19,6.41Z" /></svg></span></a>'
    html.push "  </div>"
    html.push "</div>"
    $("body").append(html.join(''))

module.exports = Message

Setup = ->
  new Message()

$(document).on('turbolinks:load', Setup)
