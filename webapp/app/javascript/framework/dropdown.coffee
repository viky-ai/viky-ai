$ = require('jquery');

class Dropdown
  constructor: ->
    $("body").on 'click', (event) => @dispatch(event)

  dispatch: (event) ->
    behavior = @get_link_with_behaviors(event).data('behavior')
    behaviors = []
    if behavior
      behaviors.push(b.trim()) for b in  behavior.split(',')
      for behavior in behaviors
        switch behavior
          when "populate-input"
            @populate_input(event)
          when "submit-form"
            @submit_form(event)
          when "replace-trigger-label"
            @replace_trigger_label(event)
          when "trigger-event"
            @trigger_event(event)
    else
      dropdown = $(event.target).closest('.dropdown')
      if dropdown.length == 1
        if $(event.target).closest('.dropdown__trigger').length == 1
          event.preventDefault()
        @toggle_dropdown(dropdown)

  toggle_dropdown: (dropdown) ->
    @setup_overlay(dropdown)
    dropdown_content = dropdown.find('.dropdown__content')
    if dropdown_content.hasClass('dropdown__content--hidden')
      @open(dropdown)
    else
      @close(dropdown)

  open: (dropdown) ->
    dropdown.find('.dropdown__content').removeClass('dropdown__content--hidden')
    dropdown.find('.dropdown__overlay').show()
    dropdown.find('.dropdown__content').removeClass('dropdown__content--on-top')
    criteria = $('body').height() - dropdown.offset().top - dropdown.height() - dropdown.find(".dropdown__content").height()
    dropdown.find('.dropdown__content').addClass('dropdown__content--on-top') if criteria < 0



  close: (dropdown) ->
    dropdown.find('.dropdown__content').addClass('dropdown__content--hidden')
    dropdown.find('.dropdown__overlay').hide()

  setup_overlay: (dropdown)->
    if dropdown.find('.dropdown__overlay').length == 0
      html = "<div class='dropdown__overlay' style='display:none;'>"
      dropdown.append(html);

  get_link_with_behaviors: (event) ->
    if $(event.target).is('a')
      return $(event.target)
    else
      return $(event.target).closest('a')

  # Behaviors
  populate_input: (event) ->
    event.preventDefault()
    node = @get_link_with_behaviors(event)
    input_selector = node.data('input-selector')
    input_value = node.data('input-value')
    $(input_selector).val(input_value)
    @close(node.closest('.dropdown'))

  replace_trigger_label: (event) ->
    event.preventDefault()
    node = @get_link_with_behaviors(event)
    replace_with = node.data('replace-with')
    replace_selector = node.data('replace-selector')
    btn = node.closest('.dropdown').find(".dropdown__trigger #{replace_selector}")
    btn.html(replace_with)
    # Materialize current option
    node.closest('ul').find('a').removeClass('current')
    node.addClass('current')
    @close(node.closest('.dropdown'))

  submit_form: (event) ->
    event.preventDefault()
    node = @get_link_with_behaviors(event)
    form_selector = node.data('form-selector')

    if $(form_selector).data('remote')
      form = document.querySelector(form_selector);
      Rails.fire(form, 'submit')
    else
      action = $(form_selector).attr('action')
      params = $(form_selector).serialize()
      Turbolinks.visit("#{action}?#{params}")

  trigger_event: (event) ->
    node = @get_link_with_behaviors(event)
    event_to_fire = node.data('trigger-event')
    $('body').trigger(event_to_fire)

Setup = ->
  new Dropdown()

$(document).on('turbolinks:load', Setup)
