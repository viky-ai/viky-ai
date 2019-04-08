require 'test_helper'
require 'model_test_helper'

class EntityTest < ActiveSupport::TestCase

  test 'Basic entity creation & entities_list association' do
    assert_equal 2, entities_lists(:weather_conditions).entities.count
    entity = Entity.new(
      solution: "{gender: 'male'}",
      auto_solution_enabled: false,
      terms: [
        { term: 'Jacques', locale: 'fr' },
        { term: 'James', locale: 'en' }
      ],
      position: 11,
      entities_list: entities_lists(:weather_conditions)
    )
    assert entity.save
    assert_equal "{gender: 'male'}", entity.solution
    assert !entity.auto_solution_enabled
    assert_equal 2, entity.terms.size
    assert_equal 'Jacques', entity.terms.first['term']
    assert_equal 'fr', entity.terms.first['locale']
    assert_equal 'James', entity.terms.last['term']
    assert_equal 'en', entity.terms.last['locale']
    assert_equal entities_lists(:weather_conditions).id, entity.entities_list.id
    assert_equal 11, entity.position
    assert_equal 3, entities_lists(:weather_conditions).entities.count
    assert_equal 3, entities_lists(:weather_conditions).entities_count
  end


  test 'term and entities_list are mandatory' do
    entity = Entity.new
    assert !entity.save
    expected = {
      terms: ["can't be blank"],
      solution: ["can't be blank"],
      entities_list: ['must exist'],
    }
    assert_equal expected, entity.errors.messages
    assert_nil entity.solution
  end


  test 'Check entities_list solution and terms length' do
    entity = entities(:weather_sunny)
    entity.solution = 'a' * 2001
    entity.auto_solution_enabled = false
    entity.terms = 'a' * 5001
    assert entity.invalid?
    expected = {
      solution: ['is too long (maximum is 2000 characters)'],
      terms: ['is too long (maximum is 5000 characters)']
    }
    assert_equal expected, entity.errors.messages
  end


  test 'Entity terms length in bytes' do
    entity = entities(:weather_sunny)
    entity.terms = [
      { 'term' => 'À' * 2000, 'locale' => 'en' },
    ]
    assert entity.invalid?
    expected = {
      terms: ['(3.906 KB) is too long (maximum is 2 KB)']
    }
    assert_equal expected, entity.errors.messages
  end


  test 'Set position on new entity' do
    entity = Entity.new(
      terms: [{ term: 'Jacques', locale: 'fr' }],
      entities_list: entities_lists(:weather_conditions)
    )
    assert entity.save
    assert_equal 2, entity.position
  end


  test 'Add a term to an entity and update locales' do
    entity = entities(:weather_sunny)
    expected = [
      { 'term' => 'sun', 'locale' => 'en' },
      { 'term' => 'soleil', 'locale' => 'fr' },
    ]
    assert_equal expected, entity.terms
    entity.terms << { 'term' => 'Sol', 'locale' => 'es' }
    assert entity.save
    expected = [
      { 'term' => 'sun', 'locale' => 'en' },
      { 'term' => 'soleil', 'locale' => 'fr' },
      { 'term' => 'Sol','locale' => 'es' }
    ]
    assert_equal expected, entity.terms
  end


  test 'Can pass a list of terms as string to an entity' do
    entity = Entity.new(
      entities_list: entities_lists(:weather_conditions),
      terms: "Jacques\nJames"
    )
    assert entity.save
    expected = [
      { 'term' => 'Jacques', 'locale' => '*' },
      { 'term' => 'James', 'locale' => '*' }
    ]
    assert_equal expected, entity.terms
  end


  test 'Terms locales' do
    entity = Entity.new(
      auto_solution_enabled: false,
      entities_list: entities_lists(:weather_conditions)
    )
    entity.terms = "soleil:fr\nsun:en"
    assert entity.save
    expected = [
      { 'term' => 'soleil', 'locale' => 'fr' },
      { 'term' => 'sun', 'locale' => 'en' }
    ]
    assert_equal expected, entity.terms

    entity.terms = "soleil:\nsun"
    assert entity.save
    expected = [
      { 'term' => 'soleil', 'locale' => '*' },
      { 'term' => 'sun', 'locale' => '*' }
    ]
    assert_equal expected, entity.terms

    entity.terms = "soleil:xy"
    assert_not entity.save
    expected = {
      terms: ["uses an unauthorized locale 'xy' for this agent"]
    }
    assert_equal expected, entity.errors.messages

    entity.terms = ":fr"
    assert_not entity.save
    expected = {
      terms: ["can't contains only locale information"]
    }
    assert_equal expected, entity.errors.messages

    entity.terms = "soleil:en:fr"
    assert entity.save
    expected = [{ 'term' => 'soleil:en', 'locale' => 'fr' }]
    assert_equal expected, entity.terms
  end


  test 'Terms locales and agent locales update' do
    entity = Entity.new(
      auto_solution_enabled: false,
      entities_list: entities_lists(:weather_conditions)
    )
    expected = ["*", "en", "fr", "es"]
    assert_equal expected, entity.entities_list.agent.locales

    entity.terms = "soleil:pt"
    assert entity.save
    expected = ["*", "en", "fr", "es", "pt"]
    assert_equal expected, entity.entities_list.agent.locales

    entity.terms = "soleil:ar"
    assert entity.save
    expected = ["*", "en", "fr", "es", "pt", "ar"]
    assert_equal expected, entity.entities_list.agent.locales
  end


  test 'terms_to_s method' do
    entity = Entity.new(
      entities_list: entities_lists(:weather_conditions)
    )
    entity.terms = "soleil"
    assert entity.save
    assert_equal "soleil", entity.terms_to_s

    entity.terms = "soleil:"
    assert entity.save
    assert_equal "soleil", entity.terms_to_s

    entity.terms = "soleil:fr"
    assert entity.save
    assert_equal "soleil:fr", entity.terms_to_s

    entity.terms = ":\nsoleil\n:"
    assert entity.save
    assert_equal "soleil", entity.terms_to_s

    entity.terms = ":\nsoleil\n:fr\n:"
    assert !entity.save
    assert_equal "soleil\n:fr", entity.terms_to_s
  end


  test 'Update entities positions' do
    entities_list = entities_lists(:weather_conditions)
    assert entities_list.entities.destroy_all

    entity_0 = Entity.create(
      terms: 'entity_0',
      position: 0,
      entities_list: entities_list
    )
    entity_1 = Entity.create(
      terms: 'entity_1',
      position: 1,
      entities_list: entities_list
    )
    entity_2 = Entity.create(
      terms: 'entity_2',
      position: 2,
      entities_list: entities_list
    )

    new_positions = [entity_1.id, entity_2.id, entity_0.id, '132465789']
    Entity.update_positions(entities_list, new_positions)
    force_reset_model_cache([entity_0, entity_1, entity_2])
    assert_equal [1_000_003, 1_000_002, 1_000_001], [entity_1.position, entity_2.position, entity_0.position]

    new_positions = [entity_0.id, entity_1.id, entity_2.id]
    Entity.update_positions(entities_list, new_positions)
    force_reset_model_cache([entity_0, entity_1, entity_2])
    assert_equal [2, 1, 0], [entity_0.position, entity_1.position, entity_2.position]
  end


  test 'search' do
    assert_equal 4, Entity.count
    assert_equal 1, Entity.search('Soleil').count
    expected = [
      {"term" => "sun", "locale" => "en"},
      {"term" => "soleil", "locale" => "fr"}
    ]
    assert_equal expected, Entity.search('Soleil').first.terms
    assert_equal 0, Entity.search('Missing').count
  end


  test 'Limit maximum number of words in a term entity' do
    entity = entities(:weather_sunny)
    entity.terms = [
      { term: 'Jacques', locale: 'fr' },
      { term: (['a'] * 37).join(' '), locale: 'en' }
    ]
    assert_not entity.save
    expected = {
      terms: ['is too long (maximum is 36 elements), found: 37']
    }
    assert_equal expected, entity.errors.messages

    entity.terms = 'a1.2' * 15
    assert_not entity.save
    expected = {
      terms: ['is too long (maximum is 36 elements), found: 60']
    }
    assert_equal expected, entity.errors.messages

    entity.terms = ([
      '²', # G_UNICODE_OTHER_NUMBER (No)
      '_', # G_UNICODE_CONNECT_PUNCTUATION (Pc)
      '-', # G_UNICODE_DASH_PUNCTUATION (Pd)
      '(', # G_UNICODE_OPEN_PUNCTUATION (Ps)
      ')', # G_UNICODE_CLOSE_PUNCTUATION (Pe)
      '«', # G_UNICODE_INITIAL_PUNCTUATION (Pi)
      '”', # G_UNICODE_FINAL_PUNCTUATION (Pf)
      '&', # G_UNICODE_OTHER_PUNCTUATION (Po)
      '€', # G_UNICODE_CURRENCY_SYMBOL (Sc)
      '^', # G_UNICODE_MODIFIER_SYMBOL (Sk)
      '=', # G_UNICODE_MATH_SYMBOL (Sm)
      '©', # G_UNICODE_OTHER_SYMBOL (So)
      '力', # G_UNICODE_BREAK_IDEOGRAPHIC (ID)
      '😀', # G_UNICODE_BREAK_EMOJI_BASE (EB)
      '🏿',  # G_UNICODE_BREAK_EMOJI_MODIFIER (EM)
    ] * 3).join
    assert_not entity.save
    expected = {
      terms: ['is too long (maximum is 36 elements), found: 45']
    }
    assert_equal expected, entity.errors.messages

    entity.terms = (['a'] * 36 + ['؀' * 2]).join(' ')
    assert entity.save
  end
end
