# Marked (Markdown parsing)
marked = require('marked')

class ReadmeForm
  constructor: ->
    @cm = null
    $('body').on 'modal:load', (event) =>
      @setup() if $("#modal_container .js-readme-form").length == 1

  setup: ->
    options = {
      autofocus: true,
      lineNumbers: false,
      mode: "markdown",
      tabSize: 2,
      insertSoftTab: true,
      lineWrapping: true,
      extraKeys:
        'Tab': (editor) -> editor.execCommand 'insertSoftTab'
    }
    @cm = CodeMirror.fromTextArea($('#readme_content')[0], options)
    @cm.on 'changes', () => @update()
    @update()

  update: ->
    $('.markdown-editor__preview').html(
      marked(@cm.getValue())
    )
    $("body").trigger 'code:highlight'

Setup = ->
  if $('body').data('controller-name') == "agents"
    new ReadmeForm()

$(document).on('turbolinks:load', Setup)
