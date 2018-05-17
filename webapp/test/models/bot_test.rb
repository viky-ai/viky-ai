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

  test 'Filter available bots' do
    current_agent = agents(:weather)
    assert Bot.create(
      name: "My awesome bot_1",
      endpoint: "http://my-awesome-bot.com",
      agent: current_agent,
      wip_enabled: true
    )

    assert Bot.create(
      name: "My awesome bot_2",
      endpoint: "http://my-awesome-bot.com",
      agent: current_agent,
      wip_enabled: false
    )

    edit_user = users(:edit_on_agent_weather)
    assert_equal 3, current_agent.accessible_bots(edit_user).count
    expected = ["Weather", "My awesome bot_1", "My awesome bot_2"].sort
    assert_equal expected, current_agent.accessible_bots(edit_user).collect(&:name).sort

    show_user = users(:show_on_agent_weather)
    assert_equal 2, current_agent.accessible_bots(show_user).count
    expected = ["Weather", "My awesome bot_2"].sort
    assert_equal expected, current_agent.accessible_bots(show_user).collect(&:name).sort
  end
end
