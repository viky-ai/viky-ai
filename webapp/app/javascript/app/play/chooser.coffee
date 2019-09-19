class PlayChooser
  constructor: ->
    $('body').on 'modal:load', (event) => @setup()

  setup: ->
    $(".modal").on 'click', (event) => @dispatch(event)
    @i18n_status_one  = $('.play__modal-footer__state').data('i18n-status-one')
    @i18n_status_more = $('.play__modal-footer__state').data('i18n-status-more')
    @i18n_status_max  = $('.play__modal-footer__state').data('i18n-status-max')
    @update_state()

  dispatch: (event) ->
    agent = @get_target(event)
    action = agent.data('action')

    if action == 'play-select-agent'
      id = agent.data('id')
      btn = agent.find('.btn.btn--checkbox')
      if btn.hasClass('btn--checkbox-on')
        btn.removeClass('btn--checkbox-on').addClass('btn--checkbox-off')
        $("#input__#{id}").remove()
        $("#search__input__#{id}").remove()
      else
        if not @disable
          btn.removeClass('btn--checkbox-off').addClass('btn--checkbox-on')
          input = "<input type='hidden' name='selection[agent_ids][]' value='#{id}' id='input__#{id}'/>"
          $("#play-chooser-selected-agents").append(input)
          input_search = "<input type='hidden' name='search[selected_ids][]' value='#{id}' id='search__input__#{id}'/>"
          $("#play-chooser-search-selected-agents").append(input_search)

      @update_state()

  update_state: ->
    max_size = 20
    size = $("#play-chooser-selected-agents input").length
    if size < 2
      $('.play__modal-footer__state').html("#{size} #{@i18n_status_one}")
    else if size == max_size
      $('.play__modal-footer__state').html("#{size} #{@i18n_status_max}")
    else
      $('.play__modal-footer__state').html("#{size} #{@i18n_status_more}")

    if size == max_size
      @disable = true
      $('.btn--checkbox.btn--checkbox-off').addClass('btn--checkbox-disabled')
      for toggle in $('.btn--checkbox.btn--checkbox-off')
        $(toggle).closest('.agent-compact--play-chooser')
          .addClass('agent-compact--play-chooser--disabled')
    else
      @disable = false
      $('.btn--checkbox.btn--checkbox-off').removeClass('btn--checkbox-disabled')
      for toggle in $('.btn--checkbox')
        $(toggle).closest('.agent-compact--play-chooser')
          .removeClass('agent-compact--play-chooser--disabled')

  get_target: (event) ->
    if $(event.target).is('.agent-compact')
      return $(event.target)
    else
      return $(event.target).closest('.agent-compact')

Setup = ->
  if $('body').data('controller-name') == "play"
    new PlayChooser()

$(document).on('turbolinks:load', Setup)
