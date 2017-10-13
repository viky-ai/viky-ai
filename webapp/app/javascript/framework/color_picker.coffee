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

        preview = link.closest('.color-picker-preview')
        if preview.length == 1
          preview.attr( "class", "color-picker-preview background-color-gradient__#{input_value}")

Setup = ->
  new ColorPicker()

$(document).on('turbolinks:load', Setup)
