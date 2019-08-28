class PlayForm
  constructor: ->
    $("body").on 'click', (event) => @dispatch(event)

  dispatch: (event) ->
    agent = @get_target(event)
    action = agent.data('action')

    if action == 'play-switch-agent'
      event.preventDefault()
      $('#play_input_agent_id').val(agent.data('agent-id'));
      Rails.fire($('.play-main__form')[0], 'submit')

  get_target: (event) ->
    if $(event.target).is('a')
      return $(event.target)
    else
      return $(event.target).closest('a')

Setup = ->
  if $('body').data('controller-name') == "play"
    new PlayForm()

$(document).on('turbolinks:load', Setup)
