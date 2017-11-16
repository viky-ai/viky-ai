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
          body: { intents: [] }
        }
      )

      assert page.has_content?('No intent found.')
      click_link "JSON"
      assert page.has_content?('intents')
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
            intents: [
              {
                "id": intents(:weather_greeting).id,
                "slug": "admin/weather/weather_greeting",
                "name": "weather_greeting",
                "score": 1.0
              }
            ]
          }
        }
      )
      assert page.has_content?('1 intent found.')

      Nlp::Interpret.any_instance.stubs('proceed').returns(
        {
          status: 200,
          body: {
            intents: [
              {
                "id": intents(:weather_greeting).id,
                "slug": "admin/weather/weather_greeting",
                "name": "weather_greeting",
                "score": 1.0,
                "explanation": {
                  "expression": {
                    "text": "Hello world",
                    "slug": "weather_greeting",
                    "highlight": "[Hello world] viki.ai",
                    "expressions": [
                      {
                        "word": "Hello"
                      },
                      {
                        "word": "world"
                      }
                    ]
                  }
                }
              }
            ]
          }
        }
      )
      all('.dropdown__trigger > .btn')[1].trigger('click')
      click_link 'Verbose ON'

      assert page.has_content?('1 intent found.')
      assert page.has_content?('Hello world viki.ai')
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
          body: { intents: [] }
        }
      )
      assert page.has_content?('No intent found.')
    end

    #
    # Add intent
    #
    click_link 'New intent'
    within('.modal') do
      assert page.has_text? 'Create a new intent'
      fill_in 'ID', with: 'my-new-intent'
      click_button 'Create'
    end
    assert page.has_content?('my-new-intent')
    assert page.has_content?('No intent found.')

    #
    # Edit intent
    #
    within '.intents-list' do
      first('.dropdown__trigger > button').trigger('click')
      click_link 'Configure'
    end

    within('.modal') do
      assert page.has_text? 'Edit intent'
      fill_in 'ID', with: 'my-new-intent-updated'
      click_button 'Update'
    end
    assert page.has_text?('my-new-intent-updated')
    assert page.has_content?('No intent found.')

    #
    # Show intent
    #
    click_link 'my-new-intent-updated'
    assert page.has_text?('admin/weather/my-new-intent-updated')
    assert page.has_content?('No intent found.')
  end


end
