$ = require('jquery');

class ModalRouter
  constructor: ->
    $("body").on 'ajax:error', (event) =>
      [data, status, xhr] = event.detail
      if data.replace_modal_content_with
        App.Modal.update(data.replace_modal_content_with)

    $("body").on 'ajax:success', (event) =>
      [data, status, xhr] = event.detail
      if data.replace_modal_content_with
        App.Modal.update(data.replace_modal_content_with)

$(document).on('turbolinks:load', -> new ModalRouter())


class Modal
  constructor: ->
    $("body").on 'click', (event) => @dispatch(event)

  dispatch: (event) ->
    node   = $(event.target)
    action = node.data('action')

    if not action?
      node = $(event.target).closest('a')
      action = node.data('action')
    if not action?
      node = $(event.target).closest('button')
      action = node.data('action')

    if action is "open-modal"
      event.preventDefault()
      modal_selector =
      @prepare($(node.data('modal-selector')).clone())

    if action is "update-remote-modal"
      event.preventDefault()
      $.ajax
        url: node.attr('href')
        complete: (data) =>
          if data.status == 403
            App.Message.alert(JSON.parse(data.responseText).message)
          else
            Modal.update(data.responseText)

    if action is "open-remote-modal"
      event.preventDefault()
      $.ajax
        url: node.attr('href')
        complete: (data) =>
          if data.status == 403
            App.Message.alert(JSON.parse(data.responseText).message)
          else
            @prepare(data.responseText)
            Modal.update(data.responseText)

    if action is "close-modal"
      event.preventDefault()
      @close()

  close: ->
    $('.app-wrapper').removeClass('modal-background-effect')
    $('.modal').hide()
    $(document).off 'keyup'
    $('body').trigger('modal:close')

  @update: (html_content) ->
    $('#modal_container').html(html_content)
    $('#modal_container .modal').show()
    $('body').trigger('modal:load')

  prepare: (html_content) ->
    $("<div id='modal_container'></div>").appendTo('body') if ($('#modal_container').length == 0)
    $('.app-wrapper').addClass('modal-background-effect')
    $(document).on 'keyup', (e) => @close() if e.keyCode == 27

module.exports = Modal

Setup = ->
  new Modal()

$(document).on('turbolinks:load', Setup)
