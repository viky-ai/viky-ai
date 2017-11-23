$ = require('jquery');

class TagAddPopup
  @show: (id, position) ->
    $('#popup-add-tag a').data('editor-id', id)
    $('#popup-add-tag').show()
    $('#popup-add-tag').css(top: position.bottom + 3, left: position.left)

  @hide: ->
    $('#popup-add-tag').hide()


class TagRemovePopup
  @show: (id, position) ->
    $('#popup-remove-tag a').data('editor-id', id)
    $('#popup-remove-tag').show()
    $('#popup-remove-tag').css(top: position.bottom + 3, left: position.left)

  @hide: ->
    $('#popup-remove-tag').hide()


class AliasesAbstract
  constructor: (element, aliases) ->
    @element = element
    @aliases = aliases

  update: ->
    @element.html('')
    html = []
    for alias in @aliases
      line = []
      css_class      = alias.data.split('____')[0]
      interpretation = alias.data.split('____')[1]
      line.push "(#{alias.start}:#{alias.end}) "
      line.push "<span class='#{css_class}'>#{interpretation}</span>"
      html.push line.join("")

    if html.length == 0
      @element.hide()
    else
      @element.html(html.join('<br/>')).show()


class InterpretationTagger
  @instances = []

  constructor: (editor_element) ->
    @alias           = []
    @deletable_alias = []
    @editor         = editor_element.editor
    @editor_id      = $(editor_element).attr("trix-id")
    @editor_element = editor_element
    InterpretationTagger.instances.push @editor_id
    @sync()

    $(document).on 'click', (event) =>
      if $(event.target).is('a')
        link = $(event.target)
      else
        link = $(event.target).closest('a')

      if link.data('editor-id') == @editor_id
        if link.data('action') == 'remove-tags'
          event.preventDefault()
          @removeTags()
        if link.data('action') == 'add-tag'
          event.preventDefault()
          @addTag(link.data('tag'))

    $(document).on 'trix-change', (event) =>
      @sync() if event.target == @editor_element

    $(document).on 'trix-selection-change', (event) =>
      if event.target == @editor_element
        range = @editor.getSelectedRange()
        if range[0] == range[1]
          TagAddPopup.hide()
          TagRemovePopup.hide()
        else
          @updateDeletableAlias(range)
          if @deletable_alias.length == 0
            TagAddPopup.show(@editor_id, @editor.getClientRectAtPosition(range[0]))
            TagRemovePopup.hide()
          else
            TagRemovePopup.show(@editor_id, @editor.getClientRectAtPosition(range[0]))
            TagAddPopup.hide()

  updateDeletableAlias: (range) =>
    @deletable_alias = []
    for alias in @alias
      if range[0] < alias.end && range[1] > alias.start && range[0] != range[1]
        @deletable_alias.push alias

  sync: =>
    @syncAlias()
    @syncAbstract()
    @syncInput()

  syncInput: =>
    input = $(@editor.element).closest('form').find('input[name="interpretation[expression]"]')
    input.val(@editor.getDocument().toString())

  syncAbstract: ->
    element = $(@editor.element).closest('form').find('.aliases')
    alias_form = new AliasesAbstract(element, @alias)
    alias_form.update()

  syncAlias: =>
    @alias = []
    position = 0
    for piece in @editor.getDocument().getPieces()
      start = position
      end   = position + piece.length
      position = end
      keys = piece.getAttributesHash().getKeys()
      if keys.length > 0 && keys[0] != 'blockBreak'
        @alias.push { start: start, end: end, data: piece.getAttributes().href }

  removeTags: ->
    for alias in @deletable_alias
      @editor.setSelectedRange([alias.start, alias.end])
      @editor.deactivateAttribute('href')
    @deletable_alias = []
    range = @editor.getSelectedRange()
    @editor.setSelectedRange([range[1], range[1]])

  addTag: (data) ->
    range = @editor.getSelectedRange()
    @editor.activateAttribute('href', data)
    @editor.setSelectedRange([range[1], range[1]])


SetupForm = ->
  if $('body').data('controller-name') == "intents" && $('body').data('controller-action') == "show"
    for trix in $("trix-editor")
      unless InterpretationTagger.instances.includes($(trix).attr("trix-id"))
        new InterpretationTagger(trix)

$(document).on('trix-initialize', SetupForm)
