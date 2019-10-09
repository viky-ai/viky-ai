class ButtonGroup
  constructor: ->
    $("body").on 'click', (event) => @dispatch(event)
    $('body').on 'btn-group:click', (event, data) => @click(data)

  behaviors: (button) ->
    behavior = button.data('behavior')
    behaviors = []
    if behavior
      behaviors.push(b.trim()) for b in  behavior.split(',')
    behaviors

  dispatch: (event) ->
    if $(event.target).is('button')
      button = $(event.target)
      group = button.closest('.btn-group')

      if group.length == 1
        event.preventDefault()
        @toggle_display(group, button)

      if button.data('behavior')
        for behavior in @behaviors(button)
          switch behavior
            when "populate-input"
              event.preventDefault()
              @populate_input(button)
            when "submit-form"
              event.preventDefault()
              @submit_form(button)
            when "trigger-event"
              @trigger_event(button)

  toggle_display: (group, selected_button) ->
    $(group).children().removeClass('btn--primary')
    $(selected_button).addClass('btn--primary')

  populate_input: (button) ->
    input_selector = button.data('input-selector')
    input_value = button.data('input-value')
    $(input_selector).val(input_value)

  submit_form: (button) ->
    form_selector = button.data('form-selector')
    if $(form_selector).data('remote')
      form = document.querySelector(form_selector);
      Rails.fire(form, 'submit')
    else
      action = $(form_selector).attr('action')
      params = $(form_selector).serialize()
      Turbolinks.visit("#{action}?#{params}")

  trigger_event: (button) ->
    event_to_fire = button.data('trigger-event')
    $('body').trigger(event_to_fire)

  # Update via JS event "btn-group:click"
  click: (data) ->
    selector = data.selector
    value    = data.on
    if selector
      for button in $(selector).find('button')
        if $(button).html().includes(value)
          node = $(button)
      if node
        @toggle_display(node.closest('.btn-group'), node)
        if node.data('behavior')
          for behavior in @behaviors(node)
            switch behavior
              when "populate-input"
                @populate_input(node)
              when "trigger-event"
                @trigger_event(node)

Setup = ->
  new ButtonGroup()

$(document).on('turbolinks:load', Setup)
