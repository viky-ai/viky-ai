$ = require('jquery');

class Dropdown
  constructor: ->
    $("body").on 'click', (event) => @dispatch(event)
    $('body').on 'dropdown:click', (event, data) => @click(data)

  behaviors: (node) ->
    behavior = node.data('behavior')
    behaviors = []
    if behavior
      behaviors.push(b.trim()) for b in  behavior.split(',')
    behaviors

  dispatch: (event) ->
    node = @get_link_with_behaviors(event)
    if node.data('behavior')
      for behavior in @behaviors(node)
        switch behavior
          when "populate-input"
            event.preventDefault()
            @populate_input(node)
          when "submit-form"
            event.preventDefault()
            @submit_form(node)
          when "replace-trigger-label"
            event.preventDefault()
            @replace_trigger_label(node)
          when "trigger-event"
            @trigger_event(node)
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

    dropdown_height =  dropdown.height() + dropdown.find(".dropdown__content").height() + 10
    exceeds_at_the_bottom = ($('body').height() - dropdown.offset().top - dropdown_height) < 0
    do_not_exceeds_at_the_top = dropdown.offset().top > dropdown_height
    if exceeds_at_the_bottom && do_not_exceeds_at_the_top
      dropdown.find('.dropdown__content').addClass('dropdown__content--on-top')

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
  populate_input: (node) ->
    input_selector = node.data('input-selector')
    input_value = node.data('input-value')
    $(input_selector).val(input_value)
    @close(node.closest('.dropdown'))

  replace_trigger_label: (node) ->
    replace_with = node.data('replace-with')
    replace_selector = node.data('replace-selector')
    btn = node.closest('.dropdown').find(".dropdown__trigger #{replace_selector}")
    btn.html(replace_with)
    # Materialize current option
    node.closest('ul').find('a').removeClass('current')
    node.addClass('current')
    @close(node.closest('.dropdown'))

  submit_form: (node) ->
    form_selector = node.data('form-selector')
    if $(form_selector).data('remote')
      form = document.querySelector(form_selector);
      Rails.fire(form, 'submit')
    else
      action = $(form_selector).attr('action')
      params = $(form_selector).serialize()
      Turbolinks.visit("#{action}?#{params}")

  trigger_event: (node) ->
    event_to_fire = node.data('trigger-event')
    $('body').trigger(event_to_fire)

  # Update via JS event "dropdown:click"
  click: (data) ->
    selector = data.selector
    value    = data.on
    if selector
      for link in $(selector).find('a')
        if $(link).data('input-value') && $(link).data('input-value').toString() == value
          node = $(link)
        if $(link).html().includes(value)
          node = $(link)
      if node
        for behavior in @behaviors(node)
          switch behavior
            when "populate-input"
              @populate_input(node)
            when "replace-trigger-label"
              @replace_trigger_label(node)
            when "trigger-event"
              @trigger_event(node)

Setup = ->
  new Dropdown()

$(document).on('turbolinks:load', Setup)
