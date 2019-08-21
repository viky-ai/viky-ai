require "application_system_test_case"

class ConsoleTest < ApplicationSystemTestCase

  test 'console basic interaction' do
    admin_go_to_agents_index
    click_link "My awesome weather bot admin/weather"

    within('.console') do
      fill_in 'interpret[sentence]', with: "hello"

      Nlp::Interpret.any_instance.stubs('send_nlp_request').returns(
        {
          status: 200,
          body: { 'interpretations' => [] }
        }
      )
      click_button 'console-send-sentence'

      assert page.has_content?('No interpretation found.')
      click_link "JSON"
      assert page.has_content?('interpretations')
    end
  end


  test 'console basic interaction with content, verbose mode, now and spellchecking' do
    admin_go_to_agents_index
    click_link "My awesome weather bot admin/weather"

    within('.console') do
      fill_in 'interpret[sentence]', with: "Hello world viki.ai"

      Nlp::Interpret.any_instance.stubs('send_nlp_request').returns(
        {
          status: 200,
          body: {
            'interpretations' => [
              {
                "id" => intents(:weather_forecast).id,
                "slug" => "admin/weather/weather_forecast",
                "name" => "weather_forecast",
                "score" => 1.0
              }
            ]
          }
        }
      )
      click_button 'console-send-sentence'
      assert page.has_content?('1 interpretation found.')

      Nlp::Interpret.any_instance.stubs('send_nlp_request').returns(
        {
          status: 200,
          body: {
            'interpretations' => [
              {
                "id" => intents(:weather_forecast).id,
                "slug" => "admin/weather/weather_forecast",
                "name" => "weather_forecast",
                "score" => 1.0,
                "explanation" => {
                  "scores" => "cov:1.00 loc:1.00 spell:1.00 olap:1.00 any:1.00 ctx:1.00 scope:0.90",
                  "highlight" => {
                    "summary" => "[Hello world] viki.ai",
                    "words" => [
                      {
                        "word" => "Hello world",
                        "match" => {
                          "expression" => "Hello world",
                          "interpretation_slug" => "weather_forecast",
                          "interpretation_id" => intents(:weather_forecast).id,
                          "expression_pos" => interpretations(:weather_forecast_tomorrow).position,
                        }
                      },
                      {
                        "word" => "viki.ai",
                        "match" => nil
                      }
                    ]
                  },
                  "expression" => {
                    "text" => "Hello world",
                    "slug" => "weather_forecast",
                    "highlight" => "[Hello world] viki.ai",
                    "expressions" => [
                      {
                        "word" => "Hello"
                      },
                      {
                        "word" => "world"
                      }
                    ]
                  }
                }
              }
            ]
          }
        }
      )
      click_button 'ON'

      assert first('button[data-input-value="true"]').matches_css?(".btn--primary")
      assert page.has_content?('1 interpretation found.')
      assert page.has_content?('Hello world viki.ai')

      # Play with Auto/Manual Datetime
      assert_equal 0, all("input[name='interpret[now]']").count
      click_button 'Manual'
      assert first('button[data-trigger-event="console-select-now-type-manual"]').matches_css?(".btn--primary")
      fill_in 'interpret[now]', with: "2017-12-05T15:14:01+01:00"
      assert_equal 1, all("input[name='interpret[now]']").count

      find('button[data-trigger-event="console-select-now-type-auto"]').click
      assert first('button[data-trigger-event="console-select-now-type-auto"]').matches_css?(".btn--primary")
      assert_equal 0, all("input[name='interpret[now]']").count

      # Spellchecking
      assert page.has_content?('Low')
      all('.dropdown__trigger > .btn')[1].click
      click_link 'High'

      assert page.has_content?('High')
      assert page.has_content?('1 interpretation found.')
      assert page.has_content?('Hello world viki.ai')
    end
  end


  test "console is persisted during agent nagivation, operation" do
    admin_go_to_agents_index
    click_link "My awesome weather bot admin/weather"

    Nlp::Interpret.any_instance.stubs('send_nlp_request').returns(
      {
        status: 200,
        body: { 'interpretations' => [] }
      }
    )

    within('.console') do
      fill_in 'interpret[sentence]', with: "hello"
      click_button 'console-send-sentence'
      assert page.has_content?('No interpretation found.')
    end

    #
    # Add intent
    #
    click_link 'Interpretations'
    click_link 'New interpretation'
    within('.modal') do
      assert page.has_text? 'Create a new interpretation'
      fill_in 'ID', with: 'my-new-intent'
      click_button 'Create'
    end
    assert page.has_content?('my-new-intent')
    assert page.has_content?('No interpretation found.')

    #
    # Edit intent
    #
    within '#intents-list-is_public' do
      first('.dropdown__trigger > button').click
      click_link 'Configure'
    end

    within('.modal') do
      assert page.has_text? 'Edit interpretation'
      fill_in 'ID', with: 'my-new-intent-updated'
      click_button 'Update'
    end
    assert page.has_text?('my-new-intent-updated')
    assert page.has_content?('No interpretation found.')

    #
    # Show intent
    #
    click_link 'my-new-intent-updated'
    assert page.has_text?('Interpretations / my-new-intent-updated PUBLIC')
    assert page.has_content?('No interpretation found.')
  end


  test "Highlighted text should be clickable and should show the matched interpretations" do
    admin_go_to_agents_index
    click_link "My awesome weather bot admin/weather"

    within('.console') do
      fill_in 'interpret[sentence]', with: "weather"

      Nlp::Interpret.any_instance.stubs('send_nlp_request').returns(
        {
          status: 200,
          body: {
            'interpretations' => [
              {
                "id" => intents(:weather_forecast).id,
                "slug" => "admin/weather/weather_forecast",
                "name" => "weather_forecast",
                "score" => 1.0,
                "explanation" => {
                  "scores" => "cov:1.00 loc:1.00 spell:1.00 olap:1.00 any:1.00 ctx:1.00 scope:0.90",
                  "highlight" => {
                    "summary" => "[weather]",
                    "words" => [
                      {
                        "word" => "weather",
                        "match" => {
                          "expression" => "weather",
                          "interpretation_slug" => "admin/weather/interpretations/weather_forecast",
                          "interpretation_id" => intents(:weather_forecast).id,
                          "expression_pos" => interpretations(:weather_forecast_tomorrow).position,
                        }
                      }
                    ]
                  },
                  "expression" => {
                    "text" => "weather",
                    "slug" => "admin/weather/interpretations/weather_forecast",
                    "highlight" => "[weather]",
                    "expressions" => [
                      {
                        "word" => "weather"
                      }
                    ]
                  }
                }
              }
            ]
          }
        }
      )
      click_button 'ON'
      assert first('button[data-input-value="true"]').matches_css?(".btn--primary")
      assert page.has_content?('1 interpretation found.')
      assert page.has_content?('weather')

      within('.c-intent__highlight') do
        # Show highlight
        page.find('.highlight-words', text: 'weather').click
        assert page.has_link?('admin/weather/interpretations/weather_forecast')
        click_link('admin/weather/interpretations/weather_forecast')

        # Hide highlight
        page.find('.highlight-words', text: 'weather').click
        assert_not page.has_link?('admin/weather/interpretations/weather_forecast')

        # Show highlight again
        page.find('.highlight-words', text: 'weather').click
        assert page.has_link?('admin/weather/interpretations/weather_forecast')

        # Navigate througth highlighted interpretation
        click_link('admin/weather/interpretations/weather_forecast')
      end
    end

    assert page.has_content?('What the weather like tomorrow ?')
  end


  test "Console is persisted when navigating between matched expressions" do
    admin_go_to_agents_index
    click_link "My awesome weather bot admin/weather"

    within('.console') do
      fill_in 'interpret[sentence]', with: "weather terminator"

      Nlp::Interpret.any_instance.stubs('send_nlp_request').returns(
        {
          status: 200,
          body: {
            'interpretations' => [
              {
                "id" => intents(:weather_forecast).id,
                "slug" => "admin/weather/weather_forecast",
                "name" => "weather_forecast",
                "score" => 1.0,
                "explanation" => {
                  "scores" => "cov:1.00 loc:1.00 spell:1.00 olap:1.00 any:1.00 ctx:1.00 scope:0.90",
                  "highlight" => {
                    "summary" => "[weather]",
                    "words" => [
                      {
                        "word" => "weather",
                        "match" => {
                          "expression" => "weather",
                          "interpretation_slug" => "admin/weather/interpretations/weather_forecast",
                          "interpretation_id" => intents(:weather_forecast).id,
                          "expression_pos" => interpretations(:weather_forecast_tomorrow).position,
                        }
                      }
                    ]
                  },
                  "expression" => {
                    "text" => "weather",
                    "slug" => "admin/weather/interpretations/weather_forecast",
                    "highlight" => "[weather]",
                    "expressions" => [
                      {
                        "word" => "weather"
                      }
                    ]
                  }
                }
              },
              {
                "id" => intents(:terminator_find).id,
                "slug" => "admin/terminator/terminator_find",
                "name" => "terminator_find",
                "score" => 1.0,
                "explanation" => {
                  "scores" => "cov:1.00 loc:1.00 spell:1.00 olap:1.00 any:1.00 ctx:1.00 scope:0.90",
                  "highlight" => {
                    "summary" => "[terminator]",
                    "words" => [
                      {
                        "word" => "terminator",
                        "match" => {
                          "expression" => "terminator",
                          "interpretation_slug" => "admin/terminator/interpretations/terminator_find",
                          "interpretation_id" => intents(:terminator_find).id,
                          "expression_pos" => interpretations(:terminator_find_sarah).position,
                        }
                      }
                    ]
                  },
                  "expression" => {
                    "text" => "terminator",
                    "slug" => "admin/terminator/interpretations/terminator_find",
                    "highlight" => "[terminator]",
                    "expressions" => [
                      {
                        "word" => "terminator"
                      }
                    ]
                  }
                }
              }
            ]
          }
        }
      )
      click_button 'ON'
      assert first('button[data-input-value="true"]').matches_css?(".btn--primary")
      assert page.has_content?('2 interpretations found.')
      assert page.has_content?('My awesome weather')

      page.find('.highlight-words', text: 'terminator').click
      assert page.has_link?('admin/terminator/interpretations/terminator_find')
      click_link('admin/terminator/interpretations/terminator_find')
    end

    assert page.has_content?('Where is Sarah Connor ?')
    within('.console') do
      assert first('button[data-input-value="true"]').matches_css?(".btn--primary")
      assert page.has_content?('2 interpretations found.')
      assert page.has_content?('My awesome weather')
    end
  end

end
