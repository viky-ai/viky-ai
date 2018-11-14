$ = require('jquery');

class Highlight

  constructor: ->
    $(".highlight-words").on "click", (event) => @showExpressions(event)

  showExpressions: (event) ->
    interpretation_id = $(event.target).parents('li').last().attr('id')
    if $(event.target).hasClass("current")
      $(event.target).removeClass("current")
      $("##{interpretation_id} ##{event.target.id}_exp_list").hide()
    else
      $("##{interpretation_id} .matched-list").hide()
      $("##{interpretation_id} .highlight-words").removeClass("current")
      $(event.target).addClass("current")
      $("##{interpretation_id} ##{event.target.id}_exp_list").show()

module.exports = Highlight
