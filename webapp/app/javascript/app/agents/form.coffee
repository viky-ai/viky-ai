$ = require('jquery');

class AgentForm
  constructor: ->
    $("body").on 'ajax:success', (event) =>
      [data, status, xhr] = event.detail
      if data.status == 422
        $("#modal_container").html(data.html).find('.modal').show()
        @setup() if $("#modal_container .js-agent-form").length == 1

    $('body').on 'modal:open', (event) =>
      @setup() if $("#modal_container .js-agent-form").length == 1

  setup: ->
    if $('.field_with_errors input').length == 0
      $('#agent_name').focus()
    else
      $('.field_with_errors input').first().focus()


Setup = ->
  if $('body').data('controller-name') == "agents"
    new AgentForm()

$(document).on('turbolinks:load', Setup)
