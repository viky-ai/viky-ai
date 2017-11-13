require "application_system_test_case"

class ConsoleTest < ApplicationSystemTestCase

  test 'User index not allowed if user is not logged in' do
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

end
