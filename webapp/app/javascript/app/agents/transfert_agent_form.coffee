$ = require('jquery');

class TransfertAgentForm
  constructor: ->
     $('body').on 'modal:load', (event) =>
      @setup() if $("#modal_container .js-agent-transfert").length == 1
      
  setup: ->
    $('#input-new-owner').selectize({
      maxItems: 1
      valueField: 'username'
      labelField: 'email'
      searchField: ['email', 'username']
      placeholder: $('#input-new-owner').data('placeholder')
      dropdownParent: 'body'
      options: []
      create: false
      render: {
        option: (item, escape) ->
          html = []
          html.push "<div>"
          html.push "  <img src='#{escape(item.image)}' />"
          html.push "  #{escape(item.email)} (#{escape(item.username)})"
          html.push "</div>"
          return html.join('')
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
              callback(res.users)
          });
    });


Setup = ->
  if $('body').data('controller-name') == "agents"
    new TransfertAgentForm()

$(document).on('turbolinks:load', Setup)
