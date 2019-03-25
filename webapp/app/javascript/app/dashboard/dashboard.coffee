$ = require('jquery');

class DashboardPreserveScroll
  scrollTop = 0

  save: ->
    element = $('main')
    scrollTop = element.scrollTop() if element.length == 1

  restore: ->
    element = $('main')
    element.scrollTop(scrollTop) if element.length == 1

$(document).on('turbolinks:before-render', -> DashboardPreserveScroll::save())
$(document).on('turbolinks:render', -> DashboardPreserveScroll::restore())
