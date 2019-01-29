import Vue from 'vue/dist/vue.esm'

class ConsoleFooter
  constructor: ->
    if $('.console-container').length == 1
      unless App.ConsoleFooter
        App.ConsoleFooter = new Vue({
          el: '#console-footer',
          data: test_suite_data
        })
    else
      App.ConsoleFooter = null

Setup = -> new ConsoleFooter()

$(document).on('turbolinks:load', Setup)
