require 'test_helper'
require 'model_test_helper'

class BotTest < ActiveSupport::TestCase

  test "basic bot validations" do
    bot = Bot.new
    assert_not bot.save
    assert_equal 3, bot.errors.full_messages.count

    bot.name = "My awesome bot"
    bot.endpoint = "http://my-awesome-bot.com"
    bot.agent = agents(:weather)

    assert bot.save
  end


  test 'Filter available bots from one agent' do
    current_agent = agents(:weather)

    assert Bot.create(
      name: "Bot 1",
      endpoint: "http://my-awesome-bot.com",
      agent: current_agent,
      wip_enabled: true
    )

    assert Bot.create(
      name: "Bot 2",
      endpoint: "http://my-awesome-bot.com",
      agent: current_agent,
      wip_enabled: false
    )

    edit_user = users(:edit_on_agent_weather)
    assert_equal 3, current_agent.accessible_bots(edit_user).count
    expected = ["Bot 1", "Bot 2", "Weather"]
    assert_equal expected, current_agent.accessible_bots(edit_user).collect(&:name).sort

    show_user = users(:show_on_agent_weather)

    assert_equal 2, current_agent.accessible_bots(show_user).count
    expected = ["Bot 2", "Weather"]
    assert_equal expected, current_agent.accessible_bots(show_user).collect(&:name).sort
  end


  test 'Sort bots by last statement from the user' do
    user1 = users(:admin)
    user2 = users(:confirmed)

    bot_a = bots(:weather_bot)
    session_user2_bot_a = ChatSession.new(bot: bot_a, user: user2, created_at: '2017-01-15 03:30:00', updated_at: '2017-01-15 03:30:00')
    assert session_user2_bot_a.save

    agent_b = create_agent("AgentB", :confirmed)
    bot_b = Bot.new(
      name: "Bot B",
      endpoint: "http://my-awesome-bot-B.com",
      agent: agent_b,
      wip_enabled: false
    )
    assert bot_b.save
    session_user1_bot_b = ChatSession.new(bot: bot_b, user: user1, created_at: '2017-01-15 03:30:00', updated_at: '2017-01-15 03:30:00')
    assert session_user1_bot_b.save
    session_user2_bot_b = ChatSession.new(bot: bot_b, user: user2, created_at: '2017-02-15 03:30:00', updated_at: '2017-02-15 03:30:00')
    assert session_user2_bot_b.save

    bots = Bot.sort_by_last_statement([bot_a, bot_b], user1)
    assert_equal 2, bots.size
    assert_equal bots[0].id, bot_a.id
    assert_equal bots[1].id, bot_b.id

    bots = Bot.sort_by_last_statement([bot_a, bot_b], user2)
    assert_equal 2, bots.size
    assert_equal bots[0].id, bot_b.id
    assert_equal bots[1].id, bot_a.id
  end


  test 'List chatbot even if no session' do
    weather_bot = bots(:weather_bot)
    new_bot = Bot.new(
      name: "Bot B",
      endpoint: "http://my-awesome-bot-B.com",
      agent: agents(:weather),
      wip_enabled: false
    )
    assert new_bot.save

    bots = Bot.sort_by_last_statement([new_bot, weather_bot], users(:admin))
    # assert_equal 2, bots.size
    assert_equal bots[0].id, weather_bot.id
    assert_equal bots[1].id, new_bot.id
  end


  test 'Available bots' do
    assert Bot.create(
      name: "Bot 1",
      endpoint: "http://my-awesome-bot.com",
      agent: agents(:weather),
      wip_enabled: true
    )

    assert Bot.create(
      name: "Bot 2",
      endpoint: "http://my-awesome-bot.com",
      agent: agents(:weather),
      wip_enabled: false
    )

    edit_user = users(:edit_on_agent_weather)
    expected = ["Bot 1", "Bot 2", "Weather"]
    assert_equal expected, Bot.accessible_bots(edit_user).collect(&:name).sort

    show_user = users(:show_on_agent_weather)
    expected = ["Bot 2", "Weather"]
    assert_equal expected, Bot.accessible_bots(show_user).collect(&:name).sort
  end
end
