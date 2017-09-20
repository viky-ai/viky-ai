# $ = require('jquery');

# class BackendUsersForm
#   constructor: ->
#     $('#search_email').on 'keyup change', ->
#       email = $('#search_email').val()
#       action = $('#new_search').attr('action')
#       params = $('#new_search').serialize()
#       Turbolinks.visit("#{action}?#{params}")


# Setup = ->
#   if $('body').data('controller-name') == "backend/users"
#     new BackendUsersForm()

# $(document).on('turbolinks:load', Setup)
