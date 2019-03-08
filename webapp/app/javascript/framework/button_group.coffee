$ = require('jquery');

class ButtonGroup
  constructor: ->
    $("body").on 'click', (event) => @dispatch(event)
    $('body').on 'buttongroup:click', (event, data) => @click(data)

  dispatch: (event) ->
    if $(event.target).is('button')
      button = $(event.target)
      group = button.closest('.btn-group')
      if group.length == 1
        event.preventDefault()
        @display(group, button)
        input_selector = button.data('input-selector')
        input_value = button.data('input-value')
        @change_value(input_selector, input_value)
        behavior = group.data('behavior')
        @submit_form(event, group) if behavior == 'submit-form'
        trigger_event = button.data('trigger-event')
        @trigger_event(button) if trigger_event

  display: (group, selected_button) ->
    $(group).children().removeClass('btn--primary')
    $(selected_button).addClass('btn--primary')

  change_value: (input_selector, input_value) ->
    $(input_selector).val(input_value)

  submit_form: (event, group) ->
    event.preventDefault()
    form_selector = group.data('form-selector')

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

  # Update via JS event "buttongroup:click"
  click: (data) ->
    selector = data.selector
    value    = data.on
    if selector
      for button in $(selector).find('button')
        if $(button).html().includes(value)
          node = $(button)
      if node
        @display(node.closest('.btn-group'), node)
        if node.data('trigger-event')
          event = node.data('trigger-event')
          $('body').trigger(event)

Setup = ->
  new ButtonGroup()

$(document).on('turbolinks:load', Setup)
