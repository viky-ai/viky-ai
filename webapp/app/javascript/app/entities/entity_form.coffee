$ = require('jquery');

class EntitySolutions
  constructor: ->
    checkbox = $("input[name='entity[auto_solution_enabled]']")
    textarea = checkbox.closest('form').find("textarea[name='entity[solution]']")
    App.CodeEditor.buildJavaScriptEditor($(textarea)[0], checkbox.is(':checked'))
    $(document).on 'click', (event) => @setupListeners(event)

  setupListeners: (event) ->
    checkbox = $(event.target).closest('form').find("input[name*='auto_solution_enabled']")
    checkbox.on 'change', (event) => @update(event)

  update: (event) ->
    checkbox = $(event.target)
    solution_container = $(checkbox.closest('form').find('.CodeMirror'))[0]
    if checkbox.is(':checked')
      solution_container.CodeMirror.setOption('readOnly', true);
    else
      solution_container.CodeMirror.setOption('readOnly', false);


Setup = ->
  if $('body').data('controller-name') == "entities_lists" && $('body').data('controller-action') == "show"
    new EntitySolutions()

$(document).on('turbolinks:load', Setup)
