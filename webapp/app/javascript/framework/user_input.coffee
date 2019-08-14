$ = require('jquery');

class UserInput
  constructor: ->
    $('body').on 'modal:load', (event) =>
      @setup() if $("#modal_container .js-user-input").length == 1

  setup: ->
    input = $('#input-user')
    max_items = if input.data('max-items') then input.data('max-items') else null
    initial_values = if input.data('initial-values') then input.data('initial-values') else []
    console.log initial_values.map((value) -> value.username)
    input.selectize({
      maxItems: max_items
      delimiter: ';'
      placeholder: input.data('placeholder')
      dropdownParent: 'body'
      valueField: 'username'
      labelField: 'username'
      items: initial_values.map((value) -> value.username)
      persist: false
      options: initial_values
      create: true
    });

Setup = ->
  new UserInput()

$(document).on('turbolinks:load', Setup)
