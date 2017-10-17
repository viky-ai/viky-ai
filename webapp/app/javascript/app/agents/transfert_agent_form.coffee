$ = require('jquery');

class TransfertAgentForm
  constructor: ->
    $("body").on 'ajax:success', (event) =>
      [data, status, xhr] = event.detail
      if data.status == 422
        @setup() if $("#modal_container .js-agent-transfert").length == 1

    $('body').on 'modal:open', (event) =>
      @setup() if $("#modal_container .js-agent-transfert").length == 1

  setup: ->
    $('#input-new-owner').selectize({
      maxItems: 1,
      valueField: 'username',
      labelField: 'email',
      searchField: ['email', 'username'],
      placeholder: $('#input-new-owner').data('placeholder')
      options: [],
      create: false,
      render: {
        option: (item, escape) ->
          return '<div>' + escape(item.email) + " (" + escape(item.username) + ')</div>'
      },
      load: (query, callback) ->
        if !query.length
          callback()
        else
          $.ajax({
            url: $('#input-new-owner').data('remote-url') + "?q=" + encodeURIComponent(query),
            type: 'GET',
            error: ->
              callback()
            success: (res) ->
              callback(res.users);
          });
    });


Setup = ->
  if $('body').data('controller-name') == "agents"
    new TransfertAgentForm()

$(document).on('turbolinks:load', Setup)
