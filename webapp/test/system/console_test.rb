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
    assert page.has_text?('Expression')
    assert page.has_content?('No intent found.')
  end


end
