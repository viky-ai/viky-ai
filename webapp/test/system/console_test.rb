require "application_system_test_case"

class ConsoleTest < ApplicationSystemTestCase

  test 'console basic interaction' do
    go_to_agents_index
    click_link "My awesome weather bot admin/weather"

    within('.console') do
      fill_in 'interpret[sentence]', with: "hello"
      first('button').click

      Nlp::Interpret.any_instance.stubs('proceed').returns(
        {
          status: 200,
          body: { interpretations: [] }
        }
      )

      assert page.has_content?('No interpretation found.')
      click_link "JSON"
      assert page.has_content?('interpretations')
    end
  end


  test 'console basic interaction with content & verbose mode' do
    go_to_agents_index
    click_link "My awesome weather bot admin/weather"

    within('.console') do
      fill_in 'interpret[sentence]', with: "Hello world viki.ai"
      first('button').click

      Nlp::Interpret.any_instance.stubs('proceed').returns(
        {
          status: 200,
          body: {
            interpretations: [
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
      assert page.has_content?('1 interpretation found.')

      Nlp::Interpret.any_instance.stubs('proceed').returns(
        {
          status: 200,
          body: {
            interpretations: [
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
                          "interpretation_id" => "6b50f2d9-2a25-493c-8b93-104968418958",
                          "expression_id" => interpretations(:weather_forecast_tomorrow).id,
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
      all('.dropdown__trigger > .btn')[1].click
      click_link 'Verbose ON'

      assert page.has_content?('Verbose ON')
      assert page.has_content?('1 interpretation found.')
      assert page.has_content?('Hello world viki.ai')

      # Play with Auto/Manual Datetime
      assert_equal 0, all("input[name='interpret[now]']").count
      all('.dropdown__trigger > .btn')[2].click
      click_link 'Manual now'
      fill_in 'interpret[now]', with: "2017-12-05T15:14:01+01:00"
      assert_equal 1, all("input[name='interpret[now]']").count

      all('.dropdown__trigger > .btn')[2].click
      click_link 'Auto now'
      assert_equal 0, all("input[name='interpret[now]']").count
    end

  end


  test "console is persisted during agent nagivation, operation" do
    go_to_agents_index
    click_link "My awesome weather bot admin/weather"

    within('.console') do
      fill_in 'interpret[sentence]', with: "hello"
      first('button').click
      Nlp::Interpret.any_instance.stubs('proceed').returns(
        {
          status: 200,
          body: { interpretations: [] }
        }
      )
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
    go_to_agents_index
    click_link "My awesome weather bot admin/weather"

    within('.console') do
      fill_in 'interpret[sentence]', with: "weather"
      all('.dropdown__trigger > .btn')[1].click
      click_link 'Verbose ON'

      Nlp::Interpret.any_instance.stubs('proceed').returns(
        {
          status: 200,
          body: {
            interpretations: [
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
                          "interpretation_id" => "6b50f2d9-2a25-493c-8b93-104968418958",
                          "expression_id" => interpretations(:weather_forecast_tomorrow).id,
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
      assert page.has_content?('Verbose ON')
      assert page.has_content?('1 interpretation found.')
      assert page.has_content?('weather')

      within('.c-intent__highlight') do
        page.find('.highlight-words', text: 'weather').click
        assert page.has_link?('admin/weather/interpretations/weather_forecast')
        click_link('admin/weather/interpretations/weather_forecast')
      end
    end

    assert page.has_content?('What the weather like tomorrow ?')
  end

  test "Console is persisted when navigating between matched expressions" do
    go_to_agents_index
    click_link "My awesome weather bot admin/weather"

    within('.console') do
      fill_in 'interpret[sentence]', with: "weather terminator"
      all('.dropdown__trigger > .btn')[1].click
      click_link 'Verbose ON'

      Nlp::Interpret.any_instance.stubs('proceed').returns(
        {
          status: 200,
          body: {
            interpretations: [
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
                          "interpretation_id" => "6b50f2d9-2a25-493c-8b93-104968418958",
                          "expression_id" => interpretations(:weather_forecast_tomorrow).id,
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
                          "interpretation_id" => "6b50f2d9-2a25-493c-8b93-104968418959",
                          "expression_id" => interpretations(:terminator_find_sarah).id,
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
      assert page.has_content?('Verbose ON')
      assert page.has_content?('2 interpretations found.')
      assert page.has_content?('My awesome weather')

      page.find('.highlight-words', text: 'terminator').click
      assert page.has_link?('admin/terminator/interpretations/terminator_find')
      click_link('admin/terminator/interpretations/terminator_find')
    end

    assert page.has_content?('Where is Sarah Connor ?')
    within('.console') do
      assert page.has_content?('Verbose ON')
      assert page.has_content?('2 interpretations found.')
      assert page.has_content?('My awesome weather')
    end
  end

end
