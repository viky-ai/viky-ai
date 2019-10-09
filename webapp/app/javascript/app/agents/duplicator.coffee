class AgentDuplicator
  constructor: ->
    $('#agent-duplicate-link').on 'click', (event) =>
      @start()

  start: ->
    $('#agent-duplicate-link').hide()
    $('#agent-duplicate-waiting').show()

  @end: ->
    $('#agent-duplicate-link').show()
    $('#agent-duplicate-waiting').hide()

Setup = ->
  if $('body').data('controller-name') == 'agents' && $('body').data('controller-action') == 'show'
    new AgentDuplicator()

$(document).on('turbolinks:load', Setup)

module.exports = AgentDuplicator
