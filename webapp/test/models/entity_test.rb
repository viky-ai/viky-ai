require 'test_helper'

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
  end


  test 'term and entities_list are mandatory' do
    entity = Entity.new
    assert !entity.save
    expected = {
      terms: ["can't be blank"],
      entities_list: ['must exist'],
    }
    assert_equal expected, entity.errors.messages
  end


  test 'Check entities_list solution and terms length' do
    entity = entities(:weather_sunny)
    entity.solution = 'a' * 2001
    entity.terms = 'a' * 5001
    assert entity.invalid?
    expected = {
      solution: ['is too long (maximum is 2000 characters)'],
      terms: ['is too long (maximum is 5000 characters)']
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


  test 'Add a term to an entity' do
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
    assert !entity.save
    expected = {
      terms: ["unknown locale 'xy'"]
    }
    assert_equal expected, entity.errors.messages

    entity.terms = ":fr"
    assert !entity.save
    expected = {
      terms: ["can't contains only locale information"]
    }
    assert_equal expected, entity.errors.messages

    entity.terms = "soleil:en:fr"
    assert entity.save
    expected = [{ 'term' => 'soleil:en', 'locale' => 'fr' }]
    assert_equal expected, entity.terms
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

end
