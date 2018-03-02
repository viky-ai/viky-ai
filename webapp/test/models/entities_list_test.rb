require 'test_helper'

class EntitiesListTest < ActiveSupport::TestCase

  test 'Basic entities_list creation & agent association' do
    assert_equal 2, agents(:weather).entities_lists.count

    entities_list = EntitiesList.new(
      listname: 'weather_towns',
      description: 'Cities all around the world',
      visibility: 'is_private',
      position: 23,
      color: 'red',
      agent: agents(:weather)
    )
    assert entities_list.save

    assert_equal 23, entities_list.position
    assert_equal 'weather_towns', entities_list.listname
    assert_equal agents(:weather).id, entities_list.agent.id
    assert_equal 3, agents(:weather).entities_lists.count
    assert EntitiesList::AVAILABLE_COLORS.one? { |color| color == entities_list.color }
    assert_equal 'is_private', entities_list.visibility
    assert !entities_list.is_public?
    assert entities_list.is_private?
  end


  test 'listname and agent are mandatory' do
    entities_list = EntitiesList.new(description: 'Hello random citizen !')
    assert !entities_list.save

    expected = {
      agent: ['must exist'],
      listname: ['is too short (minimum is 3 characters)', 'can\'t be blank'],
    }
    assert_equal expected, entities_list.errors.messages
  end


  test 'Set position on new entities list' do
    entities_list = EntitiesList.new(
      listname: 'weather_towns',
      agent: agents(:weather)
    )
    assert entities_list.save
    assert_equal 2, entities_list.position
  end


  test 'Unique listname per agent' do
    entities_list = EntitiesList.new(listname: 'hello', description: 'Hello random citizen !', agent: agents(:weather))
    assert entities_list.save
    other_entities_list = EntitiesList.new(listname: 'hello', description: 'Another way to greet you...', agent: agents(:weather))
    assert !other_entities_list.save

    expected = {
      listname: ['has already been taken']
    }
    assert_equal expected, other_entities_list.errors.messages
  end


  test 'Check listname minimal length' do
    entities_list = EntitiesList.new(listname: 'h', description: 'Hello random citizen !', agent: agents(:weather))
    assert !entities_list.save

    expected = {
      listname: ['is too short (minimum is 3 characters)']
    }
    assert_equal expected, entities_list.errors.messages
  end


  test 'Check listname is cleaned' do
    entities_list = EntitiesList.new(listname: "H3ll-#'!o", description: 'Hello random citizen !', agent: agents(:weather))
    assert entities_list.save

    assert_equal 'h3ll-o', entities_list.listname
  end


  test 'Check entities_list slug' do
    entities_list = entities_lists(:weather_conditions)
    original_listname = entities_list.listname
    entities_list.listname = 'bonjour'
    assert entities_list.save

    assert_equal entities_list.id, EntitiesList.friendly.find(original_listname).id
  end


  test 'Do not override color at creation if already set' do
    entities_list = EntitiesList.new(
      listname: 'greeting',
      color: 'blue',
      agent: agents(:weather)
    )
    assert entities_list.save
    assert_equal 'blue', entities_list.color
  end


  test 'Test entities_list slug generation' do
    entities_list = entities_lists(:weather_conditions)
    assert_equal 'admin/weather/entities_lists/weather_conditions', entities_list.slug
  end


  test 'Entities list destroy' do
    entities_list = entities_lists(:weather_conditions)
    elist_id = entities_list.id

    assert_equal 1, EntitiesList.where(id: elist_id).count
    assert entities_list.destroy
    assert_equal 0, EntitiesList.where(id: elist_id).count
  end


  test 'Test update positions' do
    agent = agents(:weather_confirmed)
    entities_list_0 = EntitiesList.create(
      listname: 'intent_0',
      position: 0,
      agent: agent
    )
    entities_list_1 = EntitiesList.create(
      listname: 'entities_list_1',
      position: 1,
      agent: agent
    )
    entities_list_2 = EntitiesList.create(
      listname: 'entities_list_2',
      position: 2,
      agent: agent
    )

    new_positions = [entities_list_1.id, entities_list_2.id, entities_list_0.id, '132465789']
    agent.update_entities_lists_positions([], new_positions)
    assert_equal [2, 1, 0], [entities_list_1.reload.position, entities_list_2.reload.position, entities_list_0.reload.position]
    assert_equal %w(is_private is_private is_private), [entities_list_1.reload.visibility, entities_list_2.reload.visibility, entities_list_0.reload.visibility]
  end


  test 'Export entities_list' do
    entities_list = entities_lists(:weather_conditions)
    csv = entities_list.to_csv
    expected = ["'Terms','Auto solution','Solution'",
                "'soleil:fr|sun:en','true','weather: sunny'",
                "'pluie:fr|rain:en','true','weather: raining'",
                ''].join("\n")
    assert_equal expected, csv
  end


  test 'Export uncompleted list' do
    entities_list = entities_lists(:weather_conditions)
    sun = entities(:weather_sunny)
    sun.terms = [{ term: 'sun', locale: Locales::ANY }]
    sun.solution = ''
    sun.save
    rain = entities(:weather_raining)
    rain.auto_solution_enabled = false
    rain.save

    csv = entities_list.to_csv
    expected = ["'Terms','Auto solution','Solution'",
                "'sun','true',''",
                "'pluie:fr|rain:en','false','weather: raining'",
                ''].join("\n")
    assert_equal expected, csv
  end

  test 'Import entities from CSV' do
    io = StringIO.new
    io << "'Terms','Auto solution','Solution'\n"
    io << "'snow','false','w: snow'\n"
    io << "'cloudy|nuageux:fr','True','weather: cloudy'\n"
    io << "\n"
    entities_import = EntitiesImport.new(build_import_params(io))
    elist = entities_lists(:weather_conditions)

    assert_equal 2, elist.entities.count
    assert elist.from_csv(entities_import)
    assert_equal 4, elist.entities.count

    snow = elist.entities.find_by_solution('w: snow')
    snow_terms = [{ 'term' => 'snow', 'locale' => '*' }]
    assert_equal snow_terms, snow.terms
    assert_equal false, snow.auto_solution_enabled
    assert_equal 'w: snow', snow.solution
    assert_equal 3, snow.position

    cloudy = elist.entities.find_by_solution('weather: cloudy')
    cloudy_terms = [{ 'term' => 'cloudy', 'locale' => '*' }, { 'term' => 'nuageux', 'locale' => 'fr' }]
    assert_equal cloudy_terms, cloudy.terms
    assert_equal true, cloudy.auto_solution_enabled
    assert_equal 'weather: cloudy', cloudy.solution
    assert_equal 2, cloudy.position
    assert_equal 6, Entity.all.count
  end


  test 'Import entities missing header' do
    io = StringIO.new
    io << "'snow','false','w: snow'\n"
    io << "'cloudy|nuageux:fr','true','weather: cloudy'\n"
    entities_import = EntitiesImport.new(build_import_params(io))
    elist = entities_lists(:weather_conditions)

    assert_equal 2, elist.entities.count
    assert !elist.from_csv(entities_import)
    assert_equal 2, elist.entities.count
    assert_equal ['Missing header'], entities_import.errors[:file]
  end


  test 'Import entities missing column' do
    io = StringIO.new
    io << "'terms','auto solution','solution'\n"
    io << "'cloudy|nuageux:fr','True','weather: cloudy'\n"
    io << "'true','w: hail'\n"
    entities_import = EntitiesImport.new(build_import_params(io))
    elist = entities_lists(:weather_conditions)

    assert_equal 2, elist.entities.count
    assert !elist.from_csv(entities_import)
    assert_equal 2, elist.entities.count
    assert_equal ['Missing column in line 2'], entities_import.errors[:file]
  end


  test 'Import entities empty terms' do
    io = StringIO.new
    io << "'Terms','Auto solution','Solution'\n"
    io << "'','true','w: hail'\n"
    io << "'cloudy|nuageux:fr','True','weather: cloudy'\n"
    entities_import = EntitiesImport.new(build_import_params(io))
    elist = entities_lists(:weather_conditions)

    assert_equal 2, elist.entities.count
    assert !elist.from_csv(entities_import)
    assert_equal 2, elist.entities.count
    assert_equal ["Validation failed: Terms can't be blank in line 1"], entities_import.errors[:file]
  end


  test 'Import entities unexpected auto solution' do
    io = StringIO.new
    io << "'Terms','Auto solution','Solution'\n"
    io << "'snow','blablabla','w: snow'\n"
    io << "'cloudy|nuageux:fr','True','weather: cloudy'\n"
    entities_import = EntitiesImport.new(build_import_params(io))
    elist = entities_lists(:weather_conditions)

    assert_equal 2, elist.entities.count
    assert !elist.from_csv(entities_import)
    assert_equal 2, elist.entities.count
    assert_equal ["Validation failed: Auto solution must be true or false in line 1"], entities_import.errors[:file]
  end


  test 'Import entities wrong separator' do
    io = StringIO.new
    io << "'Terms';'Auto solution';'Solution'\n"
    io << "'snow';'false';'w: snow'\n"
    io << "'cloudy|nuageux:fr';'True';'weather: cloudy'\n"
    entities_import = EntitiesImport.new(build_import_params(io))
    elist = entities_lists(:weather_conditions)

    assert_equal 2, elist.entities.count
    assert !elist.from_csv(entities_import)
    assert_equal 2, elist.entities.count
    assert_equal ['Missing or stray quote in line 1'], entities_import.errors[:file]
  end


  test 'Import and replace entities' do
    io = StringIO.new
    io << "'Terms','Auto solution','Solution'\n"
    io << "'snow','false','w: snow'\n"
    io << "\n"
    entities_import = EntitiesImport.new(build_import_params(io, :replace))
    elist = entities_lists(:weather_conditions)

    assert_equal 2, elist.entities.count
    assert elist.from_csv(entities_import)
    assert_equal 1, elist.entities.count

    snow = elist.entities.find_by_solution('w: snow')
    snow_terms = [{ 'term' => 'snow', 'locale' => '*' }]
    assert_equal snow_terms, snow.terms
    assert_equal false, snow.auto_solution_enabled
    assert_equal 'w: snow', snow.solution
    assert_equal 3, Entity.all.count
  end


  private

    def build_import_params(io, mode = :append)
      io.rewind
      { file: Struct.new(:tempfile, :content_type).new(io, 'text/csv'), mode: mode }
    end
end
