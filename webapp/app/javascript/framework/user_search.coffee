$ = require('jquery');

class UserSearchInput
  constructor: ->
    $('body').on 'modal:load', (event) =>
      @setup() if $("#modal_container .js-user-search").length == 1

  setup: ->
    $('#input-user-search').selectize({
      maxItems: 1
      valueField: 'username'
      labelField: 'email'
      searchField: ['email', 'username']
      placeholder: $('#input-user-search').data('placeholder')
      dropdownParent: 'body'
      hideSelected: true
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
            url: $('#input-user-search').data('remote-url') + "?q=" + encodeURIComponent(query),
            type: 'GET',
            error: ->
              callback()
            success: (res) ->
              callback(res.users)
          });
    });

Setup = ->
  new UserSearchInput()

$(document).on('turbolinks:load', Setup)
