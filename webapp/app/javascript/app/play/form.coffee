class PlayForm
  constructor: ->
    $("body").on 'change', (event) => @dispatch(event)

  dispatch: (event) ->
    if $(event.target).is('input[name="play_interpreter[agent_ids][]"]')
      Rails.fire($('.play-main__form')[0], 'submit')
      $('aside input').prop('disabled', true)


Setup = ->
  if $('body').data('controller-name') == "play"
    new PlayForm()

$(document).on('turbolinks:load', Setup)
