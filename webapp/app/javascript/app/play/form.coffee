class PlayForm
  constructor: ->
    $("body").on 'click', (event) => @dispatch(event)

  dispatch: (event) ->
    agent = @get_target(event)
    action = agent.data('action')

    if action == 'play-switch-agent'
      event.preventDefault()
      disable_with = $('.play-main__form button').data('disable-with')
      $('.play-main__form button').removeAttr('data-disable-with')
      $('#play_input_ownername').val(agent.data('ownername'));
      $('#play_input_agentname').val(agent.data('agentname'));
      Rails.fire($('.play-main__form')[0], 'submit')
      $('.play-main__form button').attr('data-disable-with', disable_with)

  get_target: (event) ->
    if $(event.target).is('a')
      return $(event.target)
    else
      return $(event.target).closest('a')

Setup = ->
  if $('body').data('controller-name') == "play"
    new PlayForm()

$(document).on('turbolinks:load', Setup)
