$ = require('jquery');
autosize = require('autosize');

class EntityForm
  constructor: (form) ->
    @form = $(form)
    @setupListeners()
    autosize($("textarea[name='entity[terms]']"));
    App.CodeEditor.buildJavaScriptEditor(@findTextareaSolution()[0], @findCheckbox().is(':checked'))
    @updateSolutionState()
    @syncSolutionWithTerms()

  setupListeners: (event) ->
    @findCheckbox().on 'change', () => @updateSolutionState()
    @findCheckbox().on 'change', () => @syncSolutionWithTerms()
    @findTerms().on 'paste', () => @syncSolutionWithTerms()
    @findTerms().on 'keyup', () => @syncSolutionWithTerms()

  updateSolutionState: () ->
    if @findCheckbox().is(':checked')
      @findSolution().CodeMirror.setOption('readOnly', true);
    else
      @findSolution().CodeMirror.setOption('readOnly', false);

  syncSolutionWithTerms: () ->
    if @findCheckbox().is(':checked')
      key = @form.data('elistname')
      value = @buildAutoSolution(key, $(@findTerms()).val())
      @findSolution().CodeMirror.setValue(value)

  buildAutoSolution: (key, rawValue) ->
    term = rawValue
      .split('\n')
      .map((term) ->
        parts = term.split(':')
        if parts.length > 2 then parts[0..-2].join(':') else parts[0]
      )
      .map((term) -> term.trim())
      .filter((term) -> term != '')
      .shift()
    if term == undefined || term == ''
      return ''
    else
      return "{\n  '" + key + "': '" + term + "'\n}"

  findTerms: ->
    return @form.find("textarea[name='entity[terms]']")

  findCheckbox: ->
    return @form.find("input[name*='auto_solution_enabled']")

  findTextareaSolution: ->
    @form.find("textarea[name='entity[solution]']")

  findSolution: ->
    return $(@form.find('.CodeMirror'))[0]

module.exports = EntityForm

Setup = ->
  if $('body').data('controller-name') == "entities_lists" && $('body').data('controller-action') == "show"
    new EntityForm('#entities-form')

$(document).on('turbolinks:load', Setup)
