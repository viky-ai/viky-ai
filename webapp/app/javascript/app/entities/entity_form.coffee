$ = require('jquery');

class EntitySolutions
  constructor: ->
    checkbox = $("input[name='entity[auto_solution_enabled]']")
    solution = checkbox.closest('form').find("textarea[name='entity[solution]']")
    if checkbox.is(':checked')
      solution.prop('disabled', true);
    else
      solution.prop('disabled', false);
    $(document).on 'click', (event) => @setupListeners(event)

  setupListeners: (event) ->
    checkbox = $(event.target).closest('form').find("input[name*='auto_solution_enabled']")
    checkbox.on 'change', (event) => @update(event)

  update: (event) ->
    checkbox = $(event.target)
    solution_container = checkbox.closest('form').find("textarea[name='entity[solution]']")
    if checkbox.is(':checked')
      solution_container.prop('disabled', true);
    else
      solution_container.prop('disabled', false);


Setup = ->
  if $('body').data('controller-name') == "entities_lists" && $('body').data('controller-action') == "show"
    new EntitySolutions()

$(document).on('turbolinks:load', Setup)
