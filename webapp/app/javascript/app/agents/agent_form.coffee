class AutoAgentOrigin
  constructor: ->
    $("body").on "Agent:injectOrigin", (event) =>
        @inject()

    $("body").on "modal:load", (event) =>
      @inject() if $(".modal form").length != 0

  inject: ->
    for form in $(".modal form")
      input = "<input type='hidden' name='origin' value='#{$('body').data('controller-action')}' />"
      $(form).append(input)


$(document).on('turbolinks:load', -> new AutoAgentOrigin())



class AgentForm
  constructor: ->
    $("body").on "ajax:before", (event) =>
      @update_delete_image() if $(event.target).hasClass('js-agent-form')

    $('body').on 'modal:load', (event) =>
      @setup() if $("#modal_container .js-agent-form").length == 1

  setup: ->
    if $('.field_with_errors input').length == 0
      App.FocusInput.atEnd('#agent_name')
    else
      if $('.field_with_errors input').last().attr('id') == 'agent_image'
        @remove_current_image()
        @display_image()
      if $('.field_with_errors input').first().attr('id') != 'agent_image'
        App.FocusInput.atEnd('.field_with_errors input')

    $(".modal").on 'click', (event) => @dispatch(event)

  update_delete_image: ->
    # Do not ask to delete current image when user update image
    upload_input = $('input[type="file"]')
    delete_image_input = $('#agent-remove-image-input')
    if upload_input.val() != '' && delete_image_input.val() == '1'
      $('#agent-remove-image-input').val('0')

  dispatch: (event) ->
    link = @get_btn_target(event)
    action = link.data('action')

    if action == 'agent-remove-image'
      event.preventDefault()
      @remove_current_image()

    if action == 'agent-select-color'
      event.preventDefault()
      @display_colorpicker()

    if action == 'agent-select-image'
      event.preventDefault()
      @display_image()

    if action == 'generate-token'
      event.preventDefault()
      $.ajax
        url: link.attr('href')
        success: (data, textStatus) ->
          $('#agent_api_token').val(data.api_token)

  get_btn_target: (event) ->
    if $(event.target).is('.btn')
      return $(event.target)
    else
      return $(event.target).closest('.btn')

  remove_current_image: ->
    $('#agent-remove-image-input').val(1)
    $('.agent-upload__destroy').hide()
    $('.agent-upload__new').show()
    $('.agent-background-header .btn-group').show()
    @display_image()

  display_image: ->
    $('.color-picker-preview').hide()
    $('.agent-image-options__image').show()
    $('[data-action="agent-select-color"').removeClass('btn--primary')
    $('[data-action="agent-select-image"').addClass('btn--primary')

  display_colorpicker: ->
    $('.color-picker-preview').show()
    $('.agent-image-options__image').hide()
    $('[data-action="agent-select-color"').addClass('btn--primary')
    $('[data-action="agent-select-image"').removeClass('btn--primary')



Setup = ->
  if $('body').data('controller-name') == "agents"
    new AgentForm()

$(document).on('turbolinks:load', Setup)
