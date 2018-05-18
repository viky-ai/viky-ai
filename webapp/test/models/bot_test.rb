require 'test_helper'

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
