class Console
  constructor: ->
    $("body").on 'click', (event) => @dispatch(event)

  dispatch: (event) ->
    link = $(event.target)
    if link.data('action') == 'console-switch-to-explain'
      event.preventDefault()
      link.closest('ul').find('a').removeClass('current')
      link.addClass('current')
      $("#console-explain").show()
      $("#console-json").hide()
      if $("#console-current-tab-input").length == 1
        $("#console-current-tab-input").val("explain")

    if link.data('action') == 'console-switch-to-json'
      event.preventDefault()
      link.closest('ul').find('a').removeClass('current')
      link.addClass('current')
      $("#console-explain").hide()
      $("#console-json").show()
      if $("#console-current-tab-input").length == 1
        $("#console-current-tab-input").val("json")

Setup = ->
  new Console()

$(document).on('turbolinks:load', Setup)
