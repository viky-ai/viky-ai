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
    $("body").on 'click submit', (event) => @dispatch(event)

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
      @preload()
      @prepare()
      Modal.update($(node.data('modal-selector')).clone())

    if action is "update-remote-modal"
      event.preventDefault()
      if node.is('a')
        $.ajax
          url: node.attr('href')
          complete: (data) =>
            if data.status == 403
              App.Message.alert(JSON.parse(data.responseText).message)
            else
              Modal.update(data.responseText)
      else if node.is('form') || node.parents('form').length > 0
        closest_form = if node.is('form') then $(node) else $(node.parents('form')[0])
        $.ajax
          url: closest_form.attr('action')
          data: closest_form.serialize()
          complete: (data) =>
            if data.status == 403
              App.Message.alert(JSON.parse(data.responseText).message)
            else
              Modal.update(data.responseText)

    if action is "open-remote-modal"
      event.preventDefault()
      $.ajax
        url: node.attr('href')
        beforeSend: =>
          @preload()
        complete: (data) =>
          if data.status == 403
            $("#modal_preloader").hide()
            App.Message.alert(JSON.parse(data.responseText).message)
          else
            @prepare()
            Modal.update(data.responseText)

    if action is "close-modal"
      event.preventDefault()
      @close()

    if action is "set-loading-modal"
      icon = '
      <span class="icon icon--spin">
        <svg xmlns="http://www.w3.org/2000/svg" xmlns:xlink="http://www.w3.org/1999/xlink"
        version="1.1" width="24" height="24" viewBox="0 0 24 24">
        <path d="M12,6V9L16,5L12,1V4A8,8 0 0,0 4,12C4,13.57 4.46,15.03 5.24,16.26L6.7,
        14.8C6.25,13.97 6,13 6,12A6,6 0 0,1 12,6M18.76,7.74L17.3,9.2C17.74,10.04 18,
        11 18,12A6,6 0 0,1 12,18V15L8,19L12,23V20A8,8 0 0,0 20,12C20,10.43 19.54,
        8.97 18.76,7.74Z" /></svg>
      </span>'
      $('.modal__main').addClass('modal__main--loading').html("#{icon} Loading...")

  close: ->
    $('.app-wrapper, nav').removeClass('modal-background-effect')
    $('.modal').hide()
    $(document).off 'keyup'
    $('body').trigger('modal:close')

  @update: (html_content) ->
    $('#modal_container').html(html_content)
    $('#modal_container .modal').show()
    $('body').trigger('modal:load')

  preload: ->
    $('.app-wrapper, nav').addClass('modal-background-effect')

    if ($('#modal_preloader').length == 0)
      $("<div id='modal_preloader'></div>").appendTo('body')
    $("#modal_preloader").show()

    $("<div id='modal_container'></div>").appendTo('body') if ($('#modal_container').length == 0)

  prepare: ->
    $("#modal_preloader").hide()
    $(document).on 'keyup', (e) => @close() if e.keyCode == 27

module.exports = Modal

Setup = ->
  new Modal()

$(document).on('turbolinks:load', Setup)
