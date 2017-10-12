$ = require('jquery');

class ColorPicker
  constructor: ->
    $("body").on 'click', (event) => @dispatch(event)

  dispatch: (event) ->
    if $(event.target).is('a')
      link = $(event.target)
      date_picker = link.closest('.color-picker')
      if date_picker.length == 1
        event.preventDefault()
        input_selector = date_picker.data('input-selector')
        input_value = link.data('input-value')
        $(input_selector).val(input_value)
        date_picker.find('a').removeClass('current')
        link.addClass('current')

Setup = ->
  new ColorPicker()

$(document).on('turbolinks:load', Setup)
