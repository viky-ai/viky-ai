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
    click_link 'New interpretation'
    within('.modal') do
      assert page.has_text? 'Create a new interpretation'
      fill_in 'ID', with: 'my-new-intent'
      click_button 'Create'
    end
    assert page.has_content?('my-new-intent')
    assert page.has_content?('No interpretation found.')

    #
    # Edit intentk
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
    assert page.has_text?('admin/weather/interpretations/my-new-intent-updated')
    assert page.has_content?('No interpretation found.')
  end
end
