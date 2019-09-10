class PlayAside
  @succeed: (data) ->
    $(".agent-compact .btn--toggle span").html("0")
    for interpretation, count of data
      $("#agent-compact-#{interpretation} .btn--toggle span").html(count)

  @failed: ->
    $(".agent-compact .btn--toggle span").html("")

module.exports = PlayAside
