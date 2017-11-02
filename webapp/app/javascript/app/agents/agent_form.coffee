$ = require('jquery');

class AgentForm
  constructor: ->
    $("body").on 'ajax:success', (event) =>
      [data, status, xhr] = event.detail
      if data.status == 422
        $("#modal_container").html(data.html).find('.modal').show()
        @setup() if $("#modal_container .js-agent-form").length == 1

    $("body").on "ajax:before", (event) =>
      @update_delete_image() if $(event.target).hasClass('js-agent-form')

    $('body').on 'modal:open', (event) =>
      @setup() if $("#modal_container .js-agent-form").length == 1

  setup: ->
    if $('.field_with_errors input').length == 0
      $('#agent_name').focus()
    else
      $('.field_with_errors input').first().focus()

    $(".modal").on 'click', (event) => @dispatch(event)

  update_delete_image: ->
    # Do not ask to delete current image when user update image
    upload_input = $('input[type="file"]')
    delete_image_input = $('#agent-remove-image-input')
    if upload_input.val() != '' && delete_image_input.val() == '1'
      $('#agent-remove-image-input').val('0')

  dispatch: (event) ->
    link = @get_link_target(event)
    action = link.data('action')
    if action == 'agent-remove-image'
      event.preventDefault()
      $('#agent-remove-image-input').val(1)
      $('.agent-upload__destroy').hide()
      $('.agent-upload__new').show()
      $('.agent-background-header .btn-group').show()

    if action == 'agent-select-color'
      event.preventDefault()
      $('.color-picker-preview').show()
      $('.agent-image-options__image').hide()
      link.closest('.btn-group').find('a').removeClass('btn--primary')
      link.addClass('btn--primary')

    if action == 'agent-select-image'
      event.preventDefault()
      $('.color-picker-preview').hide()
      $('.agent-image-options__image').show()
      link.closest('.btn-group').find('a').removeClass('btn--primary')
      link.addClass('btn--primary')

    if action == 'generate-token'
      event.preventDefault()
      $.ajax
        url: link.attr('href')
        success: (data, textStatus) ->
          $('#agent_api_token').val(data.api_token)

  get_link_target: (event) ->
    if $(event.target).is('a')
      return $(event.target)
    else
      return $(event.target).closest('a')

Setup = ->
  if $('body').data('controller-name') == "agents"
    new AgentForm()

$(document).on('turbolinks:load', Setup)
