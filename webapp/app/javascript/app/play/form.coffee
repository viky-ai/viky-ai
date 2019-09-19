class PlayForm
  constructor: ->
    $("body").on "ajax:before", (event) =>
      @textarea_height = $('#play_interpreter_text').height()

    $("body").on 'play:form-updated', (event) =>
      $('#play_interpreter_text').height(@textarea_height)

    $("body").on 'click', (event) => @dispatch(event)

  dispatch: (event) ->
    agent = @get_target(event)
    action = agent.data('action')

    if action == 'play-toggle-agent'
      checkbox = agent.find('input')
      btn = agent.find('.btn.btn--toggle')
      if checkbox.prop('checked')
        btn.removeClass('btn--toggle-on').addClass('btn--toggle-off')
        checkbox.prop('checked', false)
      else
        btn.removeClass('btn--toggle-off').addClass('btn--toggle-on')
        checkbox.prop('checked', true)
      @submit()

    if action == 'play-select-all'
      $('aside .btn.btn--toggle').removeClass('btn--toggle-off').addClass('btn--toggle-on')
      $('aside input[type="checkbox"').prop('checked', true)
      @submit()

    if action == 'play-select-none'
      $('aside .btn.btn--toggle').removeClass('btn--toggle-on').addClass('btn--toggle-off')
      $('aside input[type="checkbox"').prop('checked', false)
      @submit()

  get_target: (event) ->
    if $(event.target).is('a')
      return $(event.target)
    else if $(event.target).is('.agent-compact')
      return $(event.target)
    else
      if $(event.target).closest('.agent-compact').length == 1
        $(event.target).closest('.agent-compact')
      else
        return $(event.target)

  submit: ->
    $(".agent-compact .btn--toggle span").html("...")
    Rails.fire($('.play-main__form')[0], 'submit')

Setup = ->
  if $('body').data('controller-name') == "play"
    new PlayForm()

$(document).on('turbolinks:load', Setup)
