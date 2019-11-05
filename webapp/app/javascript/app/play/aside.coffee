class PlayAside
  @succeed: (data) ->
    $(".agent-compact .btn--toggle span").html("0")
    for formulation, count of data
      $("#agent-compact-#{formulation} .btn--toggle span").html(count)

  @failed: ->
    $(".agent-compact .btn--toggle span").html("")

module.exports = PlayAside
