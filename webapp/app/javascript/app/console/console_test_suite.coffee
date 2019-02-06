import Vue from 'vue/dist/vue.esm'

class ConsoleTestSuite
  constructor: ->
    if $('.console-container').length == 1
      unless App.ConsoleTestSuite
        App.ConsoleTestSuite = new Vue({
          el: '#console-ts',
          data: test_suite_data,
          components: {
            testDetail: {
              props:{
                expected: Object,
                got: Object
              },
              template:'<code class="language-javascript" v-html="testData"></code>',
              computed: {
                testData: () ->
                  expected = Prism.highlight(JSON.stringify(this.expected, null, 2), Prism.languages.javascript, 'javascript')
                  got = Prism.highlight(JSON.stringify(this.got, null, 2), Prism.languages.javascript, 'javascript')
                  return expected + '<br>' + got
              }
            }
          }
          methods: {
            showDetails: (testId) ->
              $("#cts-item-details-#{testId}").show()
              $("#cts-item-summary-#{testId}").hide()

            hideDetails: (testId) ->
              $("#cts-item-details-#{testId}").hide()
              $("#cts-item-summary-#{testId}").show()
          },

        })
    else
      App.ConsoleTestSuite = null


Setup = -> new ConsoleTestSuite()

$(document).on('turbolinks:load', Setup)
