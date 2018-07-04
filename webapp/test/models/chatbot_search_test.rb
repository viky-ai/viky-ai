require 'test_helper'
require 'model_test_helper'

class ChatbotSearchTest < ActiveSupport::TestCase

  test 'Search chatbot empty' do
    user = users(:admin)
    s = ChatbotSearch.new(user)
    assert_equal 3, s.options.size
    assert_equal '', s.options[:query]
    assert_not s.options[:filter_wip]
    assert_equal user.id, s.options[:user_id]
    assert s.empty?
  end

  test 'Search chatbot by name' do
    user = users(:admin)
    s = ChatbotSearch.new(user)
    assert_equal 2, Bot.search(s.options).count
    s = ChatbotSearch.new(user, query: '  rnol ')
    assert_equal 1, Bot.search(s.options).count
    expected = [
      'Arnold'
    ]
    assert_equal expected, Bot.search(s.options).all.collect(&:name)
  end

  test 'Search chatbot filter wip' do
    user = users(:admin)
    s = ChatbotSearch.new(user, filter_wip: true)
    assert_equal 1, Bot.search(s.options).count
    expected = [
      'Weather'
    ]
    assert_equal expected, Bot.search(s.options).all.collect(&:name)
  end

  test 'Search chatbot in availability list of user' do
    other_bot = Bot.new(
      name: "Weather",
      endpoint: "http://my-awesome-bot.com",
      agent: agents(:weather_confirmed),
      wip_enabled: false
    )
    assert other_bot.save

    user = users(:admin)
    s = ChatbotSearch.new(user, query: 'weather')
    assert_equal 1, Bot.search(s.options).count
    expected = [
      agents(:weather).id
    ]
    assert_equal expected, Bot.search(s.options).all.collect { |bot| bot.agent.id }
  end


  test 'Save search criteria in user state' do
    user = users(:admin)
    assert_equal user.ui_state, {}
    criteria = {
      'filter_wip' => 'true',
      'query' => 'weath'
    }
    s = ChatbotSearch.new(user, criteria)
    assert_equal 1, Bot.search(s.options).count
    assert s.save
    force_reset_model_cache(user)
    assert_equal criteria.with_indifferent_access, user.ui_state['chatbot_search']
  end
end
