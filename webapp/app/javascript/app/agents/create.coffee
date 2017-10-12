$ = require('jquery');

class AgentCreateForm
  constructor: ->
    $("body").on 'ajax:success', (event) ->
      [data, status, xhr] = event.detail
      if data.status == 422
        $("#modal_container").html(data.html).find('.modal').show()

Setup = ->
  if $('body').data('controller-name') == "agents"
    new AgentCreateForm()

$(document).on('turbolinks:load', Setup)
