class DashboardPreserveScroll
  scrollTop = 0

  save: ->
    scrollTop = $('main').scrollTop() if @we_are_in_dashboard()

  restore: ->
    $('main').scrollTop(scrollTop) if @we_are_in_dashboard()

  we_are_in_dashboard: ->
    $('.dashboard-tops').length == 1

$(document).on('turbolinks:before-render', -> DashboardPreserveScroll::save())
$(document).on('turbolinks:render', -> DashboardPreserveScroll::restore())
