class DebugSolution
  constructor: ->
    hash = $(location).attr('hash')

    if hash == "#debug-solution"
      searchParams = new URLSearchParams(window.location.search)
      type = searchParams.get("debug[type]")
      id   = searchParams.get("debug[id]")
      line = parseInt(searchParams.get("debug[line]"), 10) - 1

      if type == 'formulations'
        item = $("#formulation-#{id}")
        item.find('a')[0].click()
        $("body").on 'formulations:edit:complete formulations:show-detailed:complete', =>
          @highlight(item, $("#solution__#{id}"), line)
          $("body").off 'formulations:edit:complete'
          $("body").off 'formulations:show-detailed:complete'

      if type == 'entities'
        item = $("#entity-#{id}")
        item.find('a')[0].click()
        $("body").on 'entities:edit:complete entities:show-detailed:complete', =>
          @highlight(item, $("#solution__#{id}"), line)
          $("body").off 'entities:edit:complete'
          $("body").off 'entities:show-detailed:complete'

  highlight: (item, textarea, line) ->
    item.addClass('highlight highlight--alert')
    item[0].scrollIntoView({ block: 'center', behavior: 'smooth' })
    cm = textarea.next('.CodeMirror')[0].CodeMirror
    cm.doc.addLineClass(line, "background", "js-error")
    cm.doc.addLineClass(line, "gutter", "js-error")

Setup = ->
  new DebugSolution()

$(document).on('turbolinks:load', Setup)
