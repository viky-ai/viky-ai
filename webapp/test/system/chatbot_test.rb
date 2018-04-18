require "application_system_test_case"

class ChatbotTest < ApplicationSystemTestCase

  test 'Display a simple statement in the chat' do
    go_to_chatbots

    within('.chatbots-list') do
      first('a').click
    end

    within(".chatbot__input") do
      fill_in 'entry_content', with: 'My name is Stan'
      click_button 'Send'
    end

    within(".chatbot__discussion") do
      assert page.has_text? 'My name is Stan'
    end
  end

end
