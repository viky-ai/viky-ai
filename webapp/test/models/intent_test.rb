require 'test_helper'

class IntentTest < ActiveSupport::TestCase

  test 'Basic intent creation & agent association' do
    assert_equal 2, agents(:weather).intents.count

    intent = Intent.new(
      intentname: 'greeting',
      description: 'Hello random citizen !',
      locales: ['en'],
      visibility: 'is_private'
    )
    intent.agent = agents(:weather)
    assert intent.save

    assert_equal 2, intent.position
    assert_equal 'greeting', intent.intentname
    assert_equal agents(:weather).id, intent.agent.id
    assert_equal 3, agents(:weather).intents.count
    assert Intent::AVAILABLE_COLORS.one? { |color| color == intent.color }
    assert_equal 'is_private', intent.visibility
    assert !intent.is_public?
    assert intent.is_private?
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
    intent = Intent.new(intentname: 'hello', description: 'Hello random citizen !', locales: ['en'])
    intent.agent = agents(:weather)
    assert intent.save

    other_intent = Intent.new(intentname: 'hello', description: 'Another way to greet you...', locales: ['en'])
    other_intent.agent = agents(:weather)
    assert !other_intent.save

    expected = {
      intentname: ['has already been taken']
    }
    assert_equal expected, other_intent.errors.messages
  end


  test 'Check intentname minimal length' do
    intent = Intent.new(intentname: 'h', description: 'Hello random citizen !', locales: ['en'])
    intent.agent = agents(:weather)
    assert !intent.save

    expected = {
      intentname: ['is too short (minimum is 3 characters)']
    }
    assert_equal expected, intent.errors.messages
  end


  test 'Check intentname is cleaned' do
    intent = Intent.new(intentname: "H3ll-#'!o", description: 'Hello random citizen !', locales: ['en'])
    intent.agent = agents(:weather)
    assert intent.save

    assert_equal 'h3ll-o', intent.intentname
  end


  test 'Check intent slug' do
    intent = intents(:weather_forecast)
    original_intentname = intent.intentname
    intent.intentname = 'bonjour'
    assert intent.save

    assert_equal intent.id, Intent.friendly.find(original_intentname).id
  end


  test 'Intent destroy' do
    intent = intents(:weather_forecast)
    intent_id = intent.id

    assert_equal 1, Intent.where(id: intent_id).count
    assert intent.destroy
    assert_equal 0, Intent.where(id: intent_id).count
  end


  test 'Add a locale to an intent' do
    intent = intents(:terminator_find)

    assert_equal %w[en], intent.locales
    intent.locales << 'fr'
    assert intent.save
    assert_equal %w[en fr], intent.locales
  end


  test 'Add an unknown locale to an intent' do
    intent = intents(:weather_forecast)
    intent.locales << 'xx-XX'

    assert !intent.save
    expected = {
      locales: ["unknown 'xx-XX'"]
    }
    assert_equal expected, intent.errors.messages
  end


  test 'Remove a locale from an intent' do
    intent = intents(:weather_forecast)
    assert_equal %w[en fr], intent.locales
    intent.locales -= ['fr']
    assert intent.save
    assert_equal %w[en], intent.locales
  end


  test 'Remove an unknown locale from an intent' do
    intent = intents(:weather_forecast)
    assert_equal %w[en fr], intent.locales
    intent.locales -= ['xx-XX']
    assert intent.save
    assert_equal %w[en fr], intent.locales
  end


  test 'Remove the last locale from an intent' do
    intent = intents(:weather_forecast)
    intent.locales -= ['fr']
    assert intent.save
    intent.locales -= ['en']
    assert !intent.save
    expected = {
        locales: ['can\'t be blank']
    }
    assert_equal expected, intent.errors.messages
  end


  test 'Do not override color at creation if already set' do
    intent = Intent.new(
      intentname: 'greeting',
      locales: ['en'],
      color: 'blue'
    )
    intent.agent = agents(:weather)
    assert intent.save
    assert_equal 'blue', intent.color
  end


  test 'Test intent generation' do
    intent = intents(:weather_forecast)
    assert_equal 'admin/weather/weather_forecast', intent.slug
  end


  test 'Test update positions' do
    agent = agents(:weather_confirmed)
    intent_0 = Intent.create(
      intentname: 'intent_0',
      locales: ['en'],
      position: 0,
      agent: agent
    )
    intent_1 = Intent.create(
      intentname: 'intent_1',
      locales: ['en'],
      position: 1,
      agent: agent
    )
    intent_2 = Intent.create(
      intentname: 'intent_2',
      locales: ['en'],
      position: 2,
      agent: agent
    )

    new_positions = [intent_1.id, intent_2.id, intent_0.id, '132465789']
    agent.update_intents_positions([], new_positions)
    assert_equal [2, 1, 0], [intent_1.reload.position, intent_2.reload.position, intent_0.reload.position]
    assert_equal %w(is_private is_private is_private), [intent_1.reload.visibility, intent_2.reload.visibility, intent_0.reload.visibility]
  end


end
