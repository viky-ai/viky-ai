$ = require('jquery');

class IntentList
  constructor: ->
    $("body").on 'click', (event) => @dispatch(event)

  dispatch: (event) ->
    link = @get_link_target(event)
    action = link.data('action')

    if action == 'start-reorganize-intents'
      event.preventDefault()
      @start_sortable()

    if action == 'stop-reorganize-intents'
      event.preventDefault()
      @stop_sortable()

  start_sortable: ->
    $('.js-new-intent').hide()
    $('.js-start-reorganize-intents').hide()
    $('.js-stop-reorganize-intents').show()

    $('.intents-list__item__actions').hide()
    $('.intents-list__item__draggable').show()
    $('.js-intent-show-link').addClass('disabled')

    @sortable = Sortable.create($('#intents-list')[0], {
      handle: '.btn--drag',
      animation: 100
    });

  stop_sortable: ->
    $('.js-new-intent').show()
    $('.js-start-reorganize-intents').show()
    $('.js-stop-reorganize-intents').hide()

    $('.intents-list__item__actions').show()
    $('.intents-list__item__draggable').hide()
    $('.js-intent-show-link').removeClass('disabled')
    @sortable.destroy()

  get_link_target: (event) ->
    if $(event.target).is('a')
      return $(event.target)
    else
      return $(event.target).closest('a')

Setup = ->
  if $('body').data('controller-name') == "agents" && $('body').data('controller-action') == "show"
    new IntentList()

$(document).on('turbolinks:load', Setup)
