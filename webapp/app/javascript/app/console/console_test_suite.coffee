import Vue from 'vue/dist/vue.esm'
moment = require('moment');

class ConsoleTestSuite
  constructor: ->
    if $('.console-container').length == 1
      unless App.ConsoleTestSuite

        App.ConsoleTestSuite = new Vue({
          el: '#console-ts',
          data: test_suite_data,
          components: {
            testDetail: {
              props: ['details'],
              template: '<pre class="language-javascript"><code class="language-javascript" v-html="detailsAsJs"></code></pre>',
              computed: {
                detailsAsJs: () ->
                  details_json = JSON.stringify(this.details, null, 2)
                  return Prism.highlight(details_json, Prism.languages.javascript, 'javascript')
              },
            }
          }
          methods: {
            isSolutionSame: (expected, got) ->
              expected_json = JSON.stringify(expected.solution, null, 2)
              got_json = JSON.stringify(got.solution, null, 2)
              return expected_json == got_json

            showDetails: (testId) ->
              $("#cts-item-details-#{testId}").show()
              $("#cts-item-summary-#{testId}").hide()

            hideDetails: (testId) ->
              $("#cts-item-details-#{testId}").hide()
              $("#cts-item-summary-#{testId}").show()

            showRemoveConfirm: (event) ->
              itemDetail = $(event.target).parents('.cts-item')
              itemDetail.addClass('cts-item--behind')
              itemDetail.next('.cts-item-delete').show()

            removeTest: (index, event) ->
              csrfToken = $('meta[name="csrf-token"]').attr('content')
              deleteUrl = this.tests[index].delete_url
              $.ajax
                url: deleteUrl,
                method: "DELETE",
                headers: { "X-CSRF-TOKEN": csrfToken }
                error: (data) -> App.Message.alert(data.responseText)

            cancelRemove: (event) ->
              confirmBlock = $(event.target).parents('.cts-item-delete')
              confirmBlock.hide()
              confirmBlock.siblings('div.cts-item').removeClass('cts-item--behind')

            updateTest: (index, event) ->
              regressionCheck = this.tests[index]
              $('.panels-switch__panel').last()
                .addClass('panels-switch__panel--hide')
                .removeClass('panels-switch__panel--show')
              $("body").trigger('console-submit-form', {sentence: regressionCheck.sentence, language: regressionCheck.language, now: regressionCheck.now})
          },

        })
    else
      App.ConsoleTestSuite = null


Setup = -> new ConsoleTestSuite()

$(document).on('turbolinks:load', Setup)
