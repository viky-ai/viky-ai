require "application_system_test_case"

class ChatbotTest < ApplicationSystemTestCase

  test 'Display a simple statement in the chat' do
    go_to_chatbots
    within('.chatbots-list') do
      first('a').click
    end
    within(".chatbot__input") do
      fill_in 'statement_content', with: 'My name is Stan'
      first('button[type="submit"]').click
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


  #
  # Search
  #
  test 'Bot can be found by name' do
    go_to_chatbots
    within('.chatbots-list--for-index') do
      fill_in 'search_query', with: 'nol'
      click_button '#search'
      assert page.has_content?('Arnold')
      assert page.has_no_content?('Weather')
    end
  end

  test 'Bot filtered by release state' do
    go_to_chatbots
    within('.chatbots-list--for-index') do
      all('.dropdown__trigger > button').first.click
      click_link 'Exclude WIP'

      assert page.has_no_content?('Arnold')
      assert page.has_content?('Weather')

      all('.dropdown__trigger > button').first.click
      click_link 'Include WIP'

      assert page.has_content?('Arnold')
      assert page.has_content?('Weather')
    end
  end

  test 'Bot search without result display a message' do
    go_to_chatbots
    within('.chatbots-list--for-index') do
      fill_in 'search_query', with: 'azerty'
      click_button '#search'
      assert page.has_no_content?('Arnold')
      assert page.has_no_content?('Weather')
    end
  end

  test 'Keep chatbot search criteria' do
    go_to_chatbots
    within('.chatbots-list--for-index') do
      fill_in 'search_query', with: 'ath'
      click_button '#search'
      assert page.has_no_content?('Arnold')
      assert page.has_content?('Weather')
    end
    go_to_agents_index
    go_to_chatbots
    within('.chatbots-list--for-index') do
      assert page.has_text? 'ath'
      assert page.has_no_content?('Arnold')
      assert page.has_content?('Weather')
    end
  end

  test 'Hide WIP filter if no bot is in this state' do
    wip_bot = bots(:terminator_bot)
    wip_bot.wip_enabled = false
    assert wip_bot.save
    go_to_chatbots
    within('.chatbots-list--for-index') do
      assert page.has_no_text?('WIP')
    end
  end
end
