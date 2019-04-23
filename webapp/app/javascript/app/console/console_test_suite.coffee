import Vue from 'vue/dist/vue.esm'
moment = require('moment');
draggable = require('vuedraggable');

class ConsoleTestSuite
  constructor: ->
    if $('.console-container').length == 1
      unless App.ConsoleTestSuite

        App.ConsoleTestSuite = new Vue({
          el: '#console-ts',
          data: test_suite_data,
          components: {
            draggable,
            testDiff: {
              props: {
                test: Object
              },
              template: "
                <div>
                  <template v-for='row in test.diff_rows'>
                    <template v-if='row.label'>
                      <div class='cts-item__full__detail__label' v-html='row.label'></div>
                    </template>
                    <template v-if='row.value'>
                      <div class='cts-item__full__detail__value'>
                        <pre class='language-javascript'><code class='language-javascript' v-html='$options.filters.diffAsJs(row.value)'></code></pre>
                      </div>
                    </template>
                    <template v-if='row.error'>
                      <div class='cts-item__full__detail__error' v-html='row.error'></div>
                    </template>
                  </template>
                </div>
              ",
              filters: {
                diffAsJs: (raw_string) ->
                  json_string = JSON.stringify(raw_string, null, 2)
                  Prism.highlight(json_string, Prism.languages.javascript, 'javascript')
              },
            }
          }
          methods: {
            onPositionUpdate: (evt) ->
              csrfToken = $('meta[name="csrf-token"]').attr('content')
              $.ajax
                url: this.update_positions_url
                method: 'POST'
                headers: { "X-CSRF-TOKEN": csrfToken }
                data: {ids: this.tests.map((test) -> test.id)}

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
                error: (data) ->
                  App.Message.alert(data.responseText)
                success: (response) =>
                  this.summary = response.tests_suite.summary
                  this.tests = response.tests_suite.tests
                  App.ConsoleFooter.summary = response.tests_suite.summary

            cancelRemove: (event) ->
              confirmBlock = $(event.target).parents('.cts-item-delete')
              confirmBlock.hide()
              confirmBlock.siblings('div.cts-item').removeClass('cts-item--behind')

            updateTest: (index, event) ->
              regressionCheck = this.tests[index]
              $('.panels-switch__panel').last()
                .addClass('panels-switch__panel--hide')
                .removeClass('panels-switch__panel--show')
              $("body").trigger('console-submit-form', {
                  sentence: regressionCheck.sentence,
                  language: regressionCheck.language,
                  spellchecking: regressionCheck.spellchecking,
                  now: regressionCheck.now
                }
              )
          },

        })
    else
      App.ConsoleTestSuite = null


Setup = -> new ConsoleTestSuite()

$(document).on('turbolinks:load', Setup)
