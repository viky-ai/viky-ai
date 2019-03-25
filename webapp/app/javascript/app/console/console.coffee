$ = require('jquery');
moment = require('moment');

class ConsolePreserveScroll
  scrollTop = 0

  save: ->
    element = $('#console-output')
    scrollTop = element.scrollTop() if element.length == 1

  restore: ->
    element = $('#console-output')
    element.scrollTop(scrollTop) if element.length == 1

$(document).on('turbolinks:before-render', -> ConsolePreserveScroll::save())
$(document).on('turbolinks:render', -> ConsolePreserveScroll::restore())


class Console
  constructor: ->
    @timeout = null

    $("body").on 'click', (event) => @dispatch(event)

    @toogle_form_is_needed()
    $('#js-console-input-sentence').on 'input', => @toogle_form_is_needed()

    $("body").on 'console-select-now-type-auto', (event) =>
      $('#js-console-now-input-container').hide()
      $('#js-console-now-input-container input').val('')

    $("body").on 'console-select-now-type-manual', (event) =>
      $('#js-console-now-input-container input').val(moment().format())
      $('#js-console-now-input-container').show()
      $("#console-reset-btn").show()
      App.FocusInput.atEnd('#js-console-now-input-container input')

    $("body").on 'ajax:before', (event) =>
      if $(event.target).attr('id') == "js-console-form"
        if $(".console__output").hasClass('console__output__loading')
          $(".console__output").removeClass('console__output__loading')
          clearTimeout(@timeout) if @timeout

    $("body").on 'ajax:success', (event) =>
      if $(event.target).attr('id') == "js-console-form"
        $(".console__output").addClass('console__output__loading')
        @timeout = setTimeout ->
          $(".console__output").removeClass('console__output__loading')
        , 500
        $("#console-reset-btn").show()
        $('#console-output').scrollTop(0)

    $("body").on 'console-submit-form', (event, data) =>
      @send_interpret_request(data)

  dispatch: (event) ->
    link = @get_link_target(event)

    action = link.data('action') if link

    if action == 'console-enter-fullscreen'
      event.preventDefault()
      $('#console-enter-fullscreen').hide()
      $('#console-leave-fullscreen').show()
      $(".console-container").addClass("console-container--fullscreen")
      $(".app-wrapper").addClass("app-wrapper--without-h-nav")
      $("main").hide()
      $("nav.h-nav").hide()
      $(".toggle-console").hide()
      App.FocusInput.atEnd("input[name='interpret[sentence]']")

    if action == 'console-leave-fullscreen'
      event.preventDefault()
      $('#console-enter-fullscreen').show()
      $('#console-leave-fullscreen').hide()
      $(".console-container").removeClass("console-container--fullscreen")
      $(".app-wrapper").removeClass("app-wrapper--without-h-nav")
      $("main").show()
      $("nav.h-nav").show()
      $(".toggle-console").show()
      $("body").trigger("console:leave-fullscreen")

    if action == 'console-explain-highlighted-word'
      event.preventDefault()
      $('.c-intent__highlight ul').hide()
      if link.hasClass('current')
        $('.c-intent__highlight match').removeClass('current')
      else
        $('.c-intent__highlight match').removeClass('current')
        link.addClass('current')
        $(link.data('target')).show()

    if action == 'console-show-cts'
      event.preventDefault()
      $('.panels-switch__panel').last()
        .addClass('panels-switch__panel--show')
        .removeClass('panels-switch__panel--hide')

    if action == 'console-hide-cts'
      event.preventDefault()
      $('.panels-switch__panel').last()
        .addClass('panels-switch__panel--hide')
        .removeClass('panels-switch__panel--show')

  toogle_form_is_needed: ->
    if $('#js-console-input-sentence').val().trim() == ""
      $('#js-console-form').prop("disabled", true)
      $('#console-send-sentence').prop("disabled", true)
      $('#console-send-sentence').addClass('btn--disabled')
    else
      $('#js-console-form').prop("disabled", false)
      $('#console-send-sentence').prop("disabled", false)
      $('#console-send-sentence').removeClass('btn--disabled')

  get_link_target: (event) ->
    if $(event.target).is('a') || $(event.target).is('match')
      return $(event.target)
    else
      if $(event.target).closest('a').length == 1
        return $(event.target).closest('a')
      if $(event.target).closest('match').length == 1
        return $(event.target).closest('match')

  send_interpret_request: (data) ->
    now = if data.now? then 'Manual' else 'Auto'
    nowUpdater = { selector: '#console-btn-group-now-type', on: now }
    languageUpdater = { selector: '#console-dropdown-locale', on: data.language }
    spellcheckingUpdater = { selector: '#console-dropdown-spellchecking', on: data.spellchecking }
    $('body').trigger('btn-group:click', nowUpdater)
    $('body').trigger('dropdown:click', languageUpdater)
    $('body').trigger('dropdown:click', spellcheckingUpdater)

    $('#js-console-input-sentence').val(data.sentence)
    $('.js-language-input').val(data.language)
    $('.js-spellchecking-input').val(data.spellchecking)
    $('#js-console-now-input-container input').val(data.now) if data.now?

    @toogle_form_is_needed()

    Rails.fire($('#js-console-form')[0], 'submit')

Setup = ->
  if $('.console-container').length == 1
    new Console()

$(document).on('turbolinks:load', Setup)
