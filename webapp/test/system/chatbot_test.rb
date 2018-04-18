require "application_system_test_case"

class ChatbotTest < ApplicationSystemTestCase

  test 'Display a simple statement in the chat' do
    go_to_chatbots
    within('.chatbots-list') do
      first('a').click
    end
    within(".chatbot__input") do
      fill_in 'statement_content', with: 'My name is Stan'
      click_button 'Send'
    end
    within(".chatbot__discussion") do
      assert page.has_text? 'My name is Stan'
    end
  end


  test 'Only display finished bot and owned WIP' do
    weather_confirmed = agents(:weather_confirmed)
    weather_confirmed.visibility = 'is_public'
    assert weather_confirmed.save

    wip_bot = Bot.new(
      name:  'Bot test',
      endpoint: 'http://bot.test.pertimm.fr',
      agent: weather_confirmed,
      wip_enabled: true
    )
    assert wip_bot.save

    go_to_chatbots
    within('.chatbots-list') do
      assert page.has_text?('Weather')
      assert page.has_text?('Arnold WIP')
      assert page.has_no_text?('Bot test')
    end

    wip_bot.wip_enabled = false
    assert wip_bot.save
    go_to_chatbots
    within('.chatbots-list') do
      assert page.has_text?('Weather')
      assert page.has_text?('Arnold WIP')
      assert page.has_text?('Bot test')
    end
  end
end
