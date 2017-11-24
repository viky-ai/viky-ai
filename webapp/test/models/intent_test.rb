require 'test_helper'

class IntentTest < ActiveSupport::TestCase

  test 'Basic intent creation & agent association' do
    intent = Intent.new(
      intentname: 'greeting',
      description: 'Hello random citizen !',
      locales: ['en-US']
    )
    intent.agent = agents(:weather)
    assert intent.save

    assert_equal 1, intent.position
    assert_equal 'greeting', intent.intentname
    assert_equal agents(:weather).id, intent.agent.id
    assert_equal 2, agents(:weather).intents.count
    assert Intent::AVAILABLE_COLORS.one? { |color| color == intent.color }
  end


  test 'intentname, agent and locales are mandatory' do
    intent = Intent.new(description: 'Hello random citizen !')
    assert !intent.save

    expected = {
      agent: ['must exist'],
      intentname: ['is too short (minimum is 3 characters)', 'can\'t be blank'],
      locales: ['can\'t be blank']
    }
    assert_equal expected, intent.errors.messages
  end


  test 'Unique intentname per agent' do
    intent = Intent.new(intentname: 'hello', description: 'Hello random citizen !', locales: ['en-US'])
    intent.agent = agents(:weather)
    assert intent.save

    other_intent = Intent.new(intentname: 'hello', description: 'Another way to greet you...', locales: ['en-US'])
    other_intent.agent = agents(:weather)
    assert !other_intent.save

    expected = {
      intentname: ['has already been taken']
    }
    assert_equal expected, other_intent.errors.messages
  end


  test 'Check intentname minimal length' do
    intent = Intent.new(intentname: 'h', description: 'Hello random citizen !', locales: ['en-US'])
    intent.agent = agents(:weather)
    assert !intent.save

    expected = {
      intentname: ['is too short (minimum is 3 characters)']
    }
    assert_equal expected, intent.errors.messages
  end


  test 'Check intentname is cleaned' do
    intent = Intent.new(intentname: "H3ll-#'!o", description: 'Hello random citizen !', locales: ['en-US'])
    intent.agent = agents(:weather)
    assert intent.save

    assert_equal 'h3ll-o', intent.intentname
  end


  test 'Check intent slug' do
    intent = intents(:weather_greeting)
    original_intentname = intent.intentname
    intent.intentname = 'bonjour'
    assert intent.save

    assert_equal intent.id, Intent.friendly.find(original_intentname).id
  end


  test 'Intent destroy' do
    intent = intents(:weather_greeting)
    intent_id = intent.id

    assert_equal 1, Intent.where(id: intent_id).count
    assert intent.destroy
    assert_equal 0, Intent.where(id: intent_id).count
  end


  test 'Add a locale to an intent' do
    intent = intents(:terminator_find)

    assert_equal %w[en-US], intent.locales
    intent.locales << 'fr-FR'
    assert intent.save
    assert_equal %w[en-US fr-FR], intent.locales
  end


  test 'Add an unknown locale to an intent' do
    intent = intents(:weather_greeting)
    intent.locales << 'xx-XX'

    assert !intent.save
    expected = {
      locales: ["unknown 'xx-XX'"]
    }
    assert_equal expected, intent.errors.messages
  end


  test 'Remove a locale from an intent' do
    intent = intents(:weather_greeting)
    assert_equal %w[en-US fr-FR], intent.locales
    intent.locales -= ['fr-FR']
    assert intent.save
    assert_equal %w[en-US], intent.locales
  end


  test 'Remove an unknown locale from an intent' do
    intent = intents(:weather_greeting)
    assert_equal %w[en-US fr-FR], intent.locales
    intent.locales -= ['xx-XX']
    assert intent.save
    assert_equal %w[en-US fr-FR], intent.locales
  end


  test 'Remove the last locale from an intent' do
    intent = intents(:weather_greeting)
    intent.locales -= ['fr-FR']
    assert intent.save
    intent.locales -= ['en-US']
    assert !intent.save
    expected = {
        locales: ['can\'t be blank']
    }
    assert_equal expected, intent.errors.messages
  end


  test 'Do not override color at creation if already set' do
    intent = Intent.new(
      intentname: 'greeting',
      locales: ['en-US'],
      color: 'blue'
    )
    intent.agent = agents(:weather)
    assert intent.save
    assert_equal 'blue', intent.color
  end
end
