require 'test_helper'

class EntityTest < ActiveSupport::TestCase

  test 'Basic entity creation & entities_list association' do
    assert_equal 2, entities_lists(:weather_conditions).entities.count
    entity = Entity.new(
      solution: "{gender: 'male'}",
      auto_solution_enabled: false,
      terms: [{ term: 'Jacques' }, { term: 'James' }],
      entities_list: entities_lists(:weather_conditions)
    )
    assert entity.save
    assert_equal "{gender: 'male'}", entity.solution
    assert !entity.auto_solution_enabled
    assert_equal 2, entity.terms.size
    assert_equal 'Jacques', entity.terms.first['term']
    assert_equal 'James', entity.terms.last['term']
    assert_equal entities_lists(:weather_conditions).id, entity.entities_list.id
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
    entity.solution = (['a'] * 2001).join('')
    entity.terms = (['a'] * 5001).join('')
    assert !entity.valid?
    expected = {
      solution: ['is too long (maximum is 2000 characters)'],
      terms: ['is too long (maximum is 5000 characters)']
    }
    assert_equal expected, entity.errors.messages
  end


  test 'Add a term to an entity' do
    entity = entities(:weather_sunny)
    assert_equal [{'term' => 'soleil'}, {'term' => 'sun'}], entity.terms
    entity.terms << { 'term' => 'Sole' }
    assert entity.save
    assert_equal [{ 'term' => 'soleil' }, { 'term' => 'sun' }, { 'term' => 'Sole' }], entity.terms
  end


  test 'Can pass a list of terms as string to an entity' do
    entity = Entity.new(
      entities_list: entities_lists(:weather_conditions),
      terms: "Jacques\nJames"
    )
    assert entity.save
    expected = [{ 'term' => 'Jacques' }, { 'term' => 'James' }]
    assert_equal expected, entity.terms
  end


  test 'Trim each terms of the list' do
    entity = Entity.new(
      entities_list: entities_lists(:weather_conditions),
      terms: "   \nJacques\n  \nJames\n   "
    )
    assert entity.save
    expected = [{ 'term' => 'Jacques' }, { 'term' => 'James' }]
    assert_equal expected, entity.terms
  end
end
