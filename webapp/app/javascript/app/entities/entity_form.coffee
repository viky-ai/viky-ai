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


class EntityTermsSolutionSynchronizer
  @registerListeners: (form) ->
    checkbox = @findCheckbox(form)
    checkbox.on 'change', () => @syncSolutionWithTerms(form)
    terms = @findTerms(form)
    terms.on 'paste', () => @syncSolutionWithTerms(form)
    terms.on 'keyup', () => @syncSolutionWithTerms(form)

  @findTerms: (form) ->
    return form.find("textarea[name='entity[terms]']")

  @findCheckbox: (form) ->
    return form.find("input[name*='auto_solution_enabled']")

  @findSolution: (form) ->
    return $(form.find('.CodeMirror'))[0]

  @syncSolutionWithTerms: (form) ->
    terms = EntityTermsSolutionSynchronizer.findTerms(form)
    checkbox = EntityTermsSolutionSynchronizer.findCheckbox(form)
    solution = EntityTermsSolutionSynchronizer.findSolution(form)
    if checkbox.is(':checked')
      key = form.data('elistname')
      value = EntityTermsSolutionSynchronizer.buildAutoSolution(key, $(terms).val())
      solution.CodeMirror.setValue(value)

  @buildAutoSolution: (key, rawValue) ->
    term = rawValue
      .split('\n')
      .map((term) -> term.trim())
      .filter((term) -> term != '')
      .shift()
    if term == undefined || term == ''
      return ''
    else
      return "{\n  '" + key + "': '" + term + "'\n}"


module.exports = EntityTermsSolutionSynchronizer

Setup = ->
  if $('body').data('controller-name') == "entities_lists" && $('body').data('controller-action') == "show"
    new EntitySolutions()
    EntityTermsSolutionSynchronizer.registerListeners($('#entities-form'))

$(document).on('turbolinks:load', Setup)
