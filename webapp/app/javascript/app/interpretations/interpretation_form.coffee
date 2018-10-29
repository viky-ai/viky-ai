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

    if position_start.x > position_end.x
      ps = position_start
      pe = position_end
      position_start = pe
      position_end = ps

    rtl = $("trix-editor[trix-id='#{id}']").attr('dir') == 'rtl'

    if rtl
      $('#popup-add-tag').css(
        top: position_start.bottom + 3 - nav + scroll,
        left: position_end.left - $('#popup-add-tag').width() + 4
      )
    else
      $('#popup-add-tag').css(
        top: position_start.bottom + 3 - nav + scroll,
        left: position_start.left
      )

    $('#popup-add-tag-overlay').show()

    if rtl
      $('#highlight').css(
        top: position_start.bottom - nav + scroll,
        left: position_start.left + position_start.width,
        width: "#{(position_end.x - position_start.x + position_end.width - position_start.width)}px"
      ).show()
    else
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
    rtl = $("trix-editor[trix-id='#{id}']").attr('dir') == 'rtl'

    nav = $('nav').height()
    scroll = $('main').scrollTop()
    $('#popup-remove-tag a').data('editor-id', id)
    $('#popup-remove-tag').show()
    if rtl
      $('#popup-remove-tag').css(
        top: position.bottom + 3 - nav + scroll,
        left: position.left - $('#popup-remove-tag').width() + 4
      )
    else
      $('#popup-remove-tag').css(
        top: position.bottom + 3 - nav + scroll,
        left: position.left
      )
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
    @update_aliases()
    $('body').trigger 'aliases-form:update', [@form_container]

  aliasname: (alias) ->
    if $("##{alias.id}").length == 1
      return $($("##{alias.id} input[name*=aliasname]")).val()
    else
      return alias.aliasname.replace(new RegExp('-', 'g'), '_')

  getRegexp: (alias) ->
    id = "#{alias.id}_regex_details"
    if $("##{id}").length == 1
      return $($("##{id} input[name*=reg_exp]")).val()
    else
      return alias.reg_exp

  isValidRegex: (value) ->
    isValid = if value?.length then true else false
    try
      new RegExp(value)
    catch
      isValid = false
    return isValid

  showRegexStatus: (element) ->
    regex_td = $(element).closest('.regex_field').children('#regex_check')
    if(@isValidRegex(element.value))
      tick_icon = '<svg xmlns="http://www.w3.org/2000/svg" xmlns:xlink="http://www.w3.org/1999/xlink" version="1.1" width="24" height="24" viewBox="0 0 24 24"><path d="M21,7L9,19L3.5,13.5L4.91,12.09L9,16.17L19.59,5.59L21,7Z" /></svg>'
      valid_html = "<span class='icon icon icon--small icon--green'>#{tick_icon}</span> Valid Regex"
      regex_td.addClass('valid_text')
      regex_td.removeClass('invalid_text')
      regex_td.html(valid_html)
    else
      remove_icon = '<svg xmlns="http://www.w3.org/2000/svg" xmlns:xlink="http://www.w3.org/1999/xlink" version="1.1" width="24" height="24" viewBox="0 0 24 24"><path d="M19,6.41L17.59,5L12,10.59L6.41,5L5,6.41L10.59,12L5,17.59L6.41,19L12,13.41L17.59,19L19,17.59L13.41,12L19,6.41Z" /></svg>'
      invalid_html = "<span class='icon icon icon--small icon--red'>#{remove_icon}</span> Invalid Regex"
      regex_td.addClass('invalid_text')
      regex_td.removeClass('valid_text')
      regex_td.html(invalid_html)

  isChecked: (alias, attribute) ->
    if $("##{alias.id}").length == 1
      return $($("#" + alias.id + " input[name*=#{attribute}]")).is(':checked')
    else
      if alias[attribute] != undefined
        return alias[attribute]
      else
        return false

  previous_ids: ->
    ids = []
    for input in @form_container.find("input[name*='[id]']")
      ids.push($(input).val()) if $(input).val() != ''
    return ids

  current_ids: ->
    ids = []
    ids.push(alias.id) for alias in @aliases
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
      html.push "
        <input type='hidden' name='#{name_prefix}[aliasname]'                     value='' />
        <input type='hidden' name='#{name_prefix}[position_start]'                value='' />
        <input type='hidden' name='#{name_prefix}[position_end]'                  value='' />
        <input type='hidden' name='#{name_prefix}[interpretation_aliasable_id]'   value=' ' />
        <input type='hidden' name='#{name_prefix}[interpretation_aliasable_type]' value='' />
        <input type='hidden' name='#{name_prefix}[nature]'                        value='' />
        <input type='hidden' name='#{name_prefix}[is_list]'                       value='' />
        <input type='hidden' name='#{name_prefix}[id]'                            value='#{id}' />
        <input type='hidden' name='#{name_prefix}[_destroy]'                      value='1' />"
    @form_container.closest('form').prepend(html.join(''))


  build_line: (alias) ->
    name_prefix = "interpretation[interpretation_aliases_attributes][]"
    if alias.state == 'new' || alias.id == undefined
        alias_id_value = ""
      else
        alias_id_value = alias.id

    if alias.nature == 'type_number'
      reference_html  = "Number"
      reference_title = "Number"
    else if alias.nature == 'type_regex'
      reference_html = "Regex"
      reference_title = "Regex"
    else
      tmp = alias.slug.split("/")
      reference_html  = "<small>#{tmp[0]}/#{tmp[1]}/#{tmp[2]}/</small>#{tmp[3]}"
      reference_title = alias.slug

    is_list_checked = if @isChecked(alias, 'is_list') then 'checked' else ''
    any_enabled_checked = if @isChecked(alias, 'any_enabled') then 'checked' else ''
    reg_exp = if @getRegexp(alias)? then "value='#{@getRegexp(alias)}'" else ''

    line = []
    line.push "
    <tr id='#{alias.id}'>
      <td>
        <div class='field'>"

    if alias.aliasname_errors
      line.push "
          <div class='field_with_errors'>
            <input type='text' name='#{name_prefix}[aliasname]'      value='#{@aliasname(alias)}' />
          </div>
          #{alias.aliasname_errors}"
    else
      line.push "
          <input type='text' name='#{name_prefix}[aliasname]'        value='#{@aliasname(alias)}' />"

    line.push "
          <input type='hidden' name='#{name_prefix}[position_start]'                value='#{alias.start}' />
          <input type='hidden' name='#{name_prefix}[position_end]'                  value='#{alias.end}' />
          <input type='hidden' name='#{name_prefix}[interpretation_aliasable_id]'   value='#{alias.interpretation_aliasable_id}' />
          <input type='hidden' name='#{name_prefix}[interpretation_aliasable_type]' value='#{alias.interpretation_aliasable_type}' />
          <input type='hidden' name='#{name_prefix}[nature]'                        value='#{alias.nature}' />
          <input type='hidden' name='#{name_prefix}[id]'                            value='#{alias_id_value}' />
        </div>
      </td>"
    if alias.nature == 'type_number'
      line.push "
        <td><span class='#{alias.color}' title='#{reference_title}'>#{reference_html}</span></td>
      "
    else if alias.nature == 'type_regex'
      line.push "
        <td class='regex_field'><span class='#{alias.color}' title='#{reference_title}'>#{reference_html}</span></td>
      "
    else
      line.push "
        <td><span class='#{alias.color}' title='#{reference_title}'><a href='/agents/#{alias.slug}'>#{reference_html}</a></span></td>
      "

    if alias.nature == 'type_number' || alias.nature == 'type_regex'
      line.push "
        <td>
          <input type='hidden' name='#{name_prefix}[is_list]'     value='false' />
          <input type='hidden' name='#{name_prefix}[any_enabled]' value='false' />
        </td>"
    else
      line.push "
        <td class='options'>
          <label>
            <input type='radio' name='#{name_prefix}[is_list]' value='true' #{is_list_checked} /> List
          </label>
          &nbsp;
          <label>
            <input type='radio' name='#{name_prefix}[any_enabled]' value='true' #{any_enabled_checked} /> Any
          </label>
        </td>"

    line.push "<td>#{alias.selection}</td>"
    trix_id =  @form_container.closest('form').find('trix-editor').attr('trix-id')
    remove_icon = '<svg xmlns="http://www.w3.org/2000/svg" xmlns:xlink="http://www.w3.org/1999/xlink" version="1.1" width="24" height="24" viewBox="0 0 24 24"><path d="M19,6.41L17.59,5L12,10.59L6.41,5L5,6.41L10.59,12L5,17.59L6.41,19L12,13.41L17.59,19L19,17.59L13.41,12L19,6.41Z" /></svg>'
    line.push "<td><a href='#' data-action='remove-alias' data-editor-id='#{trix_id}' data-alias-id='#{alias.id}'><span class=\"icon icon--small\">#{remove_icon}</span></a></td>"
    line.push "</tr>"
    if alias.nature == 'type_regex'
      field_id = "#{alias.id}_regex_details"
      if alias.regex_errors
        line.push "<tr class='regex_field' id='#{field_id}'>
            <td colspan='2' class='field'>
              <div class='field_with_errors'>
                <input type='text' name='#{name_prefix}[reg_exp]' #{reg_exp} placeholder='Enter a regular expression'>
              </div>
              #{alias.regex_errors}
            </td>
          "
      else
        line.push "
          <tr class='regex_field' id='#{field_id}'>
            <td colspan='2' class='field'>
              <input type='text' name='#{name_prefix}[reg_exp]' #{reg_exp} placeholder='Enter a regular expression'>
            </td>
        "
      if @isValidRegex(@getRegexp(alias))
        tick_icon = '<svg xmlns="http://www.w3.org/2000/svg" xmlns:xlink="http://www.w3.org/1999/xlink" version="1.1" width="24" height="24" viewBox="0 0 24 24"><path d="M21,7L9,19L3.5,13.5L4.91,12.09L9,16.17L19.59,5.59L21,7Z" /></svg>'
        line.push "
          <td class='valid_text' id='regex_check'>
            <span class='icon icon icon--small icon--green'>#{tick_icon}</span>
            Valid Regex
          </td>
        "
      else
        line.push "
          <td class='invalid_text' id='regex_check'>
            <span class='icon icon icon--small icon--red'>#{remove_icon}</span>
            Invalid Regex
          </td>
        "
      line.push "
        <td colspan='2'><a href='https://regexper.com/' target='_blank'>Railroad Diagram</a></td>
        </tr>
        <tr class='spacer'><td colspan='5'></td></tr>
      "
    line.join("")

  update_aliases: ->
    html = []
    html.push "
    <table>
      <thead>
        <tr>
          <th>Parameter name</th>
          <th>Interpretation</th>
          <th>Options <a href='#' data-action='remove-all-interpretation-options'>(<em>Reset</em>)</a></th>
          <th>Selection</th>
          <th></th>
        </tr>
      </thead>
      <tbody>"
    html.push(@build_line(alias)) for alias in @aliases
    html.push "
      </tbody>
    </table>"

    @form_container.html(html.join(''))

    if @form_container.find('td').length > 0
      @form_container.show()
      if @form_container.find('.regex_field').length > 0
        $('.regex_field').on 'blur', 'input', (event) =>
          @showRegexStatus(event.target)
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

      if "#{link.data('editor-id')}" == "#{@editor_id}"
        if link.data('action') == 'remove-tags'
          event.preventDefault()
          @removeTags()
        if link.data('action') == 'add-tag'
          event.preventDefault()
          @addTag(link.data('data'))
        if link.data('action') == 'remove-alias'
          event.preventDefault()
          @removeAlias(link)

    # Used for system test
    $(@editor_element).on 'select-text', (event, start, end) =>
      @editor.setSelectedRange([start, end])

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
    pieces = @remove_firsts_spaces()
    for piece in pieces
      keys = piece.getAttributesHash().getKeys()
      start = position
      end   = position + piece.length
      position = end
      keys = piece.getAttributesHash().getKeys()
      if keys.length > 0 && keys[0] != 'blockBreak'
        data = JSON.parse(decodeURIComponent(piece.getAttributes().href))
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
    data.id = UUID()
    data.state = "new"
    range = @editor.getSelectedRange()
    @editor.activateAttribute('href', "#{JSON.stringify(data)}")
    @editor.setSelectedRange([range[1], range[1]])

  removeAlias: (aliasLink) ->
    aliasId = aliasLink.data('alias-id')
    alias = @aliases.find((item) -> item.id == aliasId)
    @aliases = @aliases.filter((item) -> item.id != alias.id)
    @syncAliasesForm()
    @editor.setSelectedRange([alias.start, alias.end])
    @editor.deactivateAttribute('href')
    @editor.setSelectedRange([alias.end, alias.end])

  remove_firsts_spaces: () ->
    result = []
    found_content = false
    for p in @editor.getDocument().getPieces()
      if found_content
        result.push(p)
      else
        if p.string.trim() != ''
          result.push(p)
          found_content = true
    return result

class InterpretationSolutions
  constructor: ->
    # Initial state on 'turbolinks:load' event
    if $("input[name='interpretation[auto_solution_enabled]']").is(':checked')
      $('.field--for-solution').hide()
    else
      $('.field--for-solution').show()

    $(document).on 'trix-initialize', (event) => @setupListeners(event)

  setupListeners: (event) ->
    auto_solution_input = $(event.target).closest('form').find("input[name*='auto_solution_enabled']")
    auto_solution_input.on 'change', (event) => @update(event)

  update: (event) ->
    auto_solution_input = $(event.target)
    solution_container = auto_solution_input.closest('form').find('.field--for-solution')
    if auto_solution_input.is(':checked')
      solution_container.hide()
    else
      solution_container.show()



class InterpretationOptions
  constructor: ->
    $('body').on 'aliases-form:update', (event, container) =>
      @update(container)

    $(document).on 'trix-initialize', (event) => @setupListeners(event)

    $(document).on 'click', (event) =>
      if $(event.target).is('a')
        link = $(event.target)
      else
        link = $(event.target).closest('a')
        if link.data('action') == 'remove-all-interpretation-options'
          event.preventDefault()
          link.closest('table').find('input[type="radio"]').prop('checked', false)
          link.hide()

  setupListeners: (event) ->
    form = $(event.target).closest('form')
    form.on 'change', (event) => @update(form)

  update: (container) ->
    action = 'hide'
    for input in container.closest('form').find('input[type="radio"]')
      action = 'show' if $(input).prop('checked')
    if action == 'hide'
      @hide_none_link(container)
    else
      @show_none_link(container)

  hide_none_link: (container) ->
    container.find('[data-action="remove-all-interpretation-options"]').hide()

  show_none_link: (container) ->
    container.find('[data-action="remove-all-interpretation-options"]').show()



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
    new InterpretationSolutions()
    new InterpretationOptions()

$(document).on('turbolinks:load', SetupPopUps)
