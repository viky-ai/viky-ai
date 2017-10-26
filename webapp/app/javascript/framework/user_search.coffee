$ = require('jquery');

class UserSearchInput
  constructor: ->
    $('body').on 'modal:open', (event) =>
      @setup() if $("#modal_container .js-user-search").length == 1

    $('body').on 'modal:update', (event) =>
      @setup() if $("#modal_container .js-user-search").length == 1

  setup: ->
    input = $('#input-user-search')
    max_items = if input.data('max-items') then input.data('max-items') else null
    initial_values = if input.data('initial-values') then input.data('initial-values') else []
    input.selectize({
      maxItems: max_items
      delimiter: ';'
      valueField: 'user_id'
      labelField: 'email'
      searchField: ['email', 'username']
      placeholder: input.data('placeholder')
      dropdownParent: 'body'
      hideSelected: true
      options: initial_values
      items: initial_values.map((value) -> value.user_id)
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
            url: input.data('remote-url') + "?q=" + encodeURIComponent(query),
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
