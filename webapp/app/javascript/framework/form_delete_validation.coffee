class FormValidationDelete
  constructor: ->
    $('body').on 'modal:load', (event) =>
      if $("#modal_container form").length == 1
        form = $("#modal_container form")
        if form.data('validation') == 'delete'
          input = form.find('input[name="validation"]')
          App.FocusInput.atEnd(input)

    $("body").on 'submit ajax:before', (event) =>
      form = $(event.target)
      if form.data('validation') == 'delete'
        input = form.find('input[name="validation"]')
        if input.length == 1
          if input.val() == "DELETE"
            input.closest('div').removeClass('field_with_errors')
            input.closest('.control').find('.help--error').hide()
            data = form.find('button.btn.btn--destructive').data('disable-btn-with')
            form.find('button.btn.btn--destructive').html(data) if data
          else
            input.closest('div').addClass('field_with_errors')
            input.closest('.control').find('.help--error').show()
            App.FocusInput.atEnd(input)
            event.preventDefault()

Setup = ->
  new FormValidationDelete()

$(document).on('turbolinks:load', Setup)
