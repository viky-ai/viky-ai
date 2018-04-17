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

end
