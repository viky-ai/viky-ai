import Vue from 'vue/dist/vue.esm'

class ConsoleTestSuite
  constructor: ->
    if $('.console-container').length == 1
      unless App.ConsoleTestSuite
        App.ConsoleTestSuite = new Vue({
          el: '#console-ts',
          data: test_suite_data
        })
    else
      App.ConsoleTestSuite = null


Setup = -> new ConsoleTestSuite()

$(document).on('turbolinks:load', Setup)
