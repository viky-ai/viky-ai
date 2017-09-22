$ = require('jquery');

class FormValidationDelete
  constructor: ->
    $("body").on 'submit', (event) =>
      form = $(event.target)
      if form.data('validation') == 'delete'
        input = form.find('input[name="validation"]')
        if input.length == 1
          if input.val() == "DELETE"
            input.closest('div').removeClass('field_with_errors')
            input.closest('.control').find('.help--error').hide()
          else
            input.closest('div').addClass('field_with_errors')
            input.closest('.control').find('.help--error').show()
            event.preventDefault()

Setup = ->
  new FormValidationDelete()

$(document).on('turbolinks:load', Setup)
