$ = require('jquery');

class UploadInput
  constructor: ->
    $('.field--upload input[type="file"]').mouseenter (event) ->
      $(this).closest('.field--upload').find('label').addClass('hover')

    $('.field--upload input[type="file"]').mouseleave (event) ->
      $(this).closest('.field--upload').find('label').removeClass('hover')

    $('.field--upload input[type="file"]').change (event) ->
      filename = this.files[0].name
      container = $(this).closest('.field--upload')
      container.find('label p').html("Selected file: <strong>" + filename + "</strong>")

$(document).on('turbolinks:load', ->
  new UploadInput()
  $("body").on('modal:load', -> new UploadInput())
)
