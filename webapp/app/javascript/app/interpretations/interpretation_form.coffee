$ = require('jquery');

# https://github.com/kelektiv/node-uuid
UUID = require('uuid/v1')

class PopupAddTag
  constructor: ->
    @items = []
    @index = -1

    $('body').on 'popup-add-tag:show', (event, editor_id, position_start, position_end) =>
      @show(editor_id, position_start, position_end)
    $('body').on 'popup-add-tag:hide', (event) => @hide()
    $('#popup-add-tag-overlay').on 'click', => @hide()
    $('#popup-add-tag input').on 'keyup change', (event) => @search(event)

  show: (id, position_start, position_end) ->
    $('#popup-add-tag ul li').addClass('enabled')

    @resetSearch()
    @resetItems()
    @addKeyListener()

    nav = $('nav').height()
    scroll = $('main').scrollTop()
    $('#popup-add-tag a').data('editor-id', id)
    $('#popup-add-tag').show()
    $('#popup-add-tag').css(
      top: position_start.bottom + 3 - nav + scroll,
      left: position_start.left
    )
    $('#popup-add-tag-overlay').show()
    $('#highlight').css(
      top: position_start.bottom - nav + scroll,
      left: position_start.left,
      width: "#{(position_end.x - position_start.x)}px"
    ).show()

    $('#popup-add-tag ul').scrollTop(0)

  hide: ->
    $('#highlight').hide()
    $('#popup-add-tag').hide()
    $('#popup-add-tag-overlay').hide()
    @removeKeyListener()

  resetItems: ->
    @items = $('#popup-add-tag ul li.enabled a')
    @items.removeClass('focus')
    @index = -1

  markNext: ->
    $("#popup-add-tag").focus()
    if(@index < @items.length - 1)
      @index++
    else
      @index = 0
    @items.removeClass('focus')
    $(@items[@index]).addClass('focus')

    @autoScroll()

  markPrevious: ->
    if @index > 0
      @index--
    else
      @index = @items.length - 1;
    @items.removeClass('focus')
    $(@items[@index]).addClass('focus')

    @autoScroll()

  autoScroll: ->
    if @index == 0
      $('#popup-add-tag ul').scrollTop(0)
    else
      height = $(@items[@index]).closest('ul').height()
      pos = $(@items[@index]).position().top + $(@items[@index]).closest('ul').scrollTop()
      if pos > height
        $(@items[@index]).closest('ul').scrollTop(pos - height)

  select: ->
    $(@items[@index]).trigger('click')

  removeKeyListener: ->
    Mousetrap.unbind('tab')
    Mousetrap.unbind('down')
    Mousetrap.unbind('up')
    Mousetrap.unbind('space')
    Mousetrap.unbind('esc')

  addKeyListener: ->
    @removeKeyListener()
    $(@items[@index]).addClass('focus')

    Mousetrap.bindGlobal 'tab', (event) ->
      event.preventDefault()
      $('#popup-add-tag input').focus()

    Mousetrap.bindGlobal 'down', (event) =>
      event.preventDefault()
      @markNext()

    Mousetrap.bindGlobal 'up', (event) =>
      event.preventDefault()
      @markPrevious()

    Mousetrap.bindGlobal 'space', (event) =>
      event.preventDefault()
      @select()

    Mousetrap.bindGlobal 'esc', (event) =>
      event.preventDefault()
      @hide()

  search: (event) ->
    unless [38, 40].includes(event.keyCode)
      query = $('#popup-add-tag input').val()
      regexp = new RegExp(query, 'i')
      for item in $('#popup-add-tag ul li')
        if regexp.test $(item).data('search')
          $(item).addClass('enabled')
        else
          $(item).removeClass('enabled')
      $('#popup-add-tag ul').scrollTop(0)
      @resetItems()

  resetSearch: ->
    $('#popup-add-tag input').val("")
    $('#popup-add-tag ul li').show()



class TagRemovePopup
  constructor: ->
    $('body').on 'popup-remove-tag:show', (event, id, position) => @show(id, position)
    $('body').on 'popup-remove-tag:hide', (event) => @hide()
    $('#popup-remove-tag-overlay').on 'click', => @hide()

  show: (id, position) ->
    nav = $('nav').height()
    scroll = $('main').scrollTop()
    $('#popup-remove-tag a').data('editor-id', id)
    $('#popup-remove-tag').show()
    $('#popup-remove-tag').css(top: position.bottom + 3 - nav + scroll, left: position.left)
    $('#popup-remove-tag-overlay').show()
    @addKeyListener()

  hide: ->
    $('#popup-remove-tag').hide()
    $('#popup-remove-tag-overlay').hide()
    $('#popup-remove-tag ul li a').removeClass('focus')
    @removeKeyListener()

  removeKeyListener: ->
    Mousetrap.unbind('down')
    Mousetrap.unbind('space')
    Mousetrap.unbind('esc')

  addKeyListener: ->
    @removeKeyListener()

    Mousetrap.bindGlobal 'down', (event) =>
      event.preventDefault()
      $('#popup-remove-tag ul li a').addClass('focus')

    Mousetrap.bindGlobal 'space', (event) =>
      event.preventDefault()
      $('#popup-remove-tag ul li a').first().trigger('click')

    Mousetrap.bindGlobal 'esc', (event) =>
      event.preventDefault()
      @hide()




class AliasesForm
  constructor: (form_container, aliases) ->
    @form_container = form_container
    @aliases = aliases

  update: ->
    @update_deletable_ids()
    @update_table()

  aliasname: (alias) ->
    if $("##{alias.alias_id}").length == 1
      return $($("##{alias.alias_id} input[name*=aliasname]")).val()
    else
      if alias.name != undefined
        return alias.name
      else
        return alias.slug.split('/')[2]

  previous_ids: ->
    ids = []
    for input in @form_container.find("input[name*='[id]']")
      ids.push($(input).val()) if $(input).val() != ''
    return ids

  current_ids: ->
    ids = []
    ids.push(alias.alias_id) for alias in @aliases
    return ids

  deletable_ids: ->
    ids = []
    for id in @previous_ids()
      ids.push(id) unless @current_ids().includes(id)
    return ids

  update_deletable_ids: ->
    html = []
    name_prefix = "interpretation[interpretation_aliases_attributes][]"
    for id in @deletable_ids()
      html.push "<input type='hidden' name='#{name_prefix}[aliasname]' value='' />"
      html.push "<input type='hidden' name='#{name_prefix}[position_start]' value='' />"
      html.push "<input type='hidden' name='#{name_prefix}[position_end]' value='' />"
      html.push "<input type='hidden' name='#{name_prefix}[intent_id]' value='' />"
      html.push "<input type='hidden' name='#{name_prefix}[id]' value='#{id}' />"
      html.push "<input type='hidden' name='#{name_prefix}[_destroy]' value='1' />"
    @form_container.closest('form').prepend(html.join(''))


  update_table: ->
    html = []
    html.push "<table>"
    html.push "<thead>"
    html.push "  <tr>"
    html.push "    <th>Parameter name</th>"
    html.push "    <th>Interpretation</th>"
    html.push "    <th>Selection</th>"
    html.push "  </tr>"
    html.push "</thead>"
    for alias in @aliases
      name_prefix = "interpretation[interpretation_aliases_attributes][]"
      line = []
      line.push "<tr>"
      line.push "  <td id='#{alias.alias_id}'>"
      line.push "    <div class='field'>"
      line.push "      <input type='text' name='#{name_prefix}[aliasname]' value='#{@aliasname(alias)}' />"
      line.push "      <input type='hidden' name='#{name_prefix}[position_start]' value='#{alias.start}' />"
      line.push "      <input type='hidden' name='#{name_prefix}[position_end]' value='#{alias.end}' />"
      line.push "      <input type='hidden' name='#{name_prefix}[intent_id]' value='#{alias.intent_id}' />"
      if alias.type == 'new'
        line.push "      <input type='hidden' name='#{name_prefix}[id]' value='' />"
      else
        line.push "      <input type='hidden' name='#{name_prefix}[id]' value='#{alias.alias_id}' />"
      line.push "    </div>"
      line.push "  </td>"
      line.push "  <td><span class='#{alias.color}'>#{alias.slug}</span></td>"
      line.push "  <td>#{alias.selection}</td>"
      line.push "</tr>"
      html.push line.join("")
    html.push "</table>"

    @form_container.html(html.join(''))

    if @form_container.find('td').length > 0
      @form_container.show()
    else
      @form_container.hide()





class InterpretationTagger
  @instances = []

  constructor: (editor_element) ->
    @aliases           = []
    @deletable_aliases = []
    @editor         = editor_element.editor
    @editor_id      = $(editor_element).attr("trix-id")
    @editor_element = editor_element
    InterpretationTagger.instances.push(@editor_id)
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
          @addTag(link.data('data'))

    $(@editor_element).on 'trix-change', (event) =>
      @sync()

    $(@editor_element).on 'trix-selection-change', (event) =>
      range = @editor.getSelectedRange()
      if range[0] == range[1]
        $('body').trigger 'popup-add-tag:hide'
        $('body').trigger 'popup-remove-tag:hide'
      else
        @updateDeletableAliases(range)
        if @deletable_aliases.length == 0
          $('body').trigger 'popup-remove-tag:hide'
          $('body').trigger 'popup-add-tag:show', [
            @editor_id,
            @editor.getClientRectAtPosition(range[0]),
            @editor.getClientRectAtPosition(range[1])
          ]
        else
          $('body').trigger 'popup-add-tag:hide'
          $('body').trigger 'popup-remove-tag:show', [
            @editor_id,
            @editor.getClientRectAtPosition(range[0])
          ]

  updateDeletableAliases: (range) =>
    @deletable_aliases = []
    for alias in @aliases
      if range[0] < alias.end && range[1] > alias.start && range[0] != range[1]
        @deletable_aliases.push alias

  sync: =>
    @syncAliases()
    @syncAliasesForm()
    @syncExpressionInput()

  syncExpressionInput: =>
    input = $(@editor.element).closest('form').find('textarea[name="interpretation[expression]"]')
    input.val(@editor.getDocument().toString())

  syncAliasesForm: ->
    form_container = $(@editor.element).closest('form').find('.aliases')
    new AliasesForm(form_container, @aliases).update()

  syncAliases: =>
    @aliases = []
    position = 0
    for piece in @editor.getDocument().getPieces()
      start = position
      end   = position + piece.length
      position = end
      keys = piece.getAttributesHash().getKeys()
      if keys.length > 0 && keys[0] != 'blockBreak'
        data = JSON.parse(piece.getAttributes().href)
        data.start     = start
        data.end       = end
        data.selection = piece.string
        @aliases.push(data)

  removeTags: ->
    for alias in @deletable_aliases
      @editor.setSelectedRange([alias.start, alias.end])
      @editor.deactivateAttribute('href')
    @deletable_aliases = []
    range = @editor.getSelectedRange()
    @editor.setSelectedRange([range[1], range[1]])

  addTag: (data) ->
    data.alias_id = UUID()
    data.type = "new"
    range = @editor.getSelectedRange()
    @editor.activateAttribute('href', "#{JSON.stringify(data)}")
    @editor.setSelectedRange([range[1], range[1]])



SetupForm = ->
  if $('body').data('controller-name') == "intents" && $('body').data('controller-action') == "show"
    for trix in $("trix-editor")
      unless InterpretationTagger.instances.includes($(trix).attr("trix-id"))
        new InterpretationTagger(trix)

$(document).on('trix-initialize', SetupForm)

SetupPopUps = ->
  if $('body').data('controller-name') == "intents" && $('body').data('controller-action') == "show"
    new PopupAddTag()
    new TagRemovePopup()

$(document).on('turbolinks:load', SetupPopUps)
