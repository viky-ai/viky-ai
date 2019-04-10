require 'test_helper'
require 'model_test_helper'

class EntitiesImportTest < ActiveSupport::TestCase


  test 'Basic import creation and entities list association' do
    entities_list = entities_lists(:weather_conditions)

    IO.copy_stream(File.join(Rails.root, 'test', 'fixtures', 'files', 'import_entities.csv'), 'temp.csv')
    entities_import = EntitiesImport.new({
      file: File.open('temp.csv'),
      mode: 'replace',
      entities_list: entities_list,
      user: users(:admin)
    })
    assert entities_import.save
    assert_equal 1, entities_list.entities_imports.count
  end


  test 'file should be present' do
    entities_import = EntitiesImport.new({
      mode: 'replace',
      entities_list: entities_lists(:weather_conditions),
      user: users(:admin)
    })
    assert entities_import.invalid?
    assert_equal ["must be present"], entities_import.errors[:file]
  end


  test 'file should be of csv format' do
    entities_list = entities_lists(:weather_conditions)

    IO.copy_stream(File.join(Rails.root, 'test', 'fixtures', 'files', 'avatar_upload.png'), 'temp.csv')
    entities_import = EntitiesImport.new({
      file: File.open('temp.csv'),
      mode: 'replace',
      entities_list: entities_list,
      user: users(:admin)
    })
    assert entities_import.invalid?
    assert_equal ["CSV format expected"], entities_import.errors[:file]
  end


  test 'no concurrent import running' do
    entities_list = entities_lists(:weather_conditions)

    IO.copy_stream(File.join(Rails.root, 'test', 'fixtures', 'files', 'import_entities.csv'), 'temp.csv')
    entities_import = EntitiesImport.new({
      file: File.open('temp.csv'),
      mode: 'replace',
      entities_list: entities_list,
      user: users(:admin)
    })
    assert entities_import.save

    IO.copy_stream(File.join(Rails.root, 'test', 'fixtures', 'files', 'import_entities.csv'), 'temp.csv')
    entities_import = EntitiesImport.new({
      file: File.open('temp.csv'),
      mode: 'replace',
      entities_list: entities_list,
      user: users(:admin)
    })
    assert_not entities_import.save
    assert_equal ["An import is already running, please wait."], entities_import.errors.full_messages
  end


  test 'file should have a valid csv extension' do
    entities_list = entities_lists(:weather_conditions)

    IO.copy_stream(File.join(Rails.root, 'test', 'fixtures', 'files', 'avatar_upload.png'), 'temp.png')
    entities_import = EntitiesImport.new({
      file: File.open('temp.png'),
      mode: 'replace',
      entities_list: entities_list,
      user: users(:admin)
    })
    assert entities_import.invalid?
    assert_equal ["CSV format expected", "CSV extension expected"], entities_import.errors[:file]
  end


  test 'Import entities from CSV' do
    elist = entities_lists(:weather_conditions)
    assert_equal ["*", "en", "fr", "es"], elist.agent.locales

    io = StringIO.new
    io << "Terms,Auto solution,Solution\n"
    io << "snow,false,\"{'w': 'snow'}\"\n"
    io << "cloudy|غائم:ar,False,\"{'weather': 'cloudy'}\"\n"
    io << "\n"

    entities_import = get_entities_import(elist, io)

    assert_equal 2, elist.entities.count
    assert entities_import.save
    assert_equal 2, entities_import.proceed
    assert_equal 4, elist.entities.count

    snow = elist.entities.find_by_solution("{'w': 'snow'}")
    snow_terms = [{ 'term' => 'snow', 'locale' => '*' }]
    assert_equal snow_terms, snow.terms
    assert_equal false, snow.auto_solution_enabled
    assert_equal "{'w': 'snow'}", snow.solution
    assert_equal 3, snow.position

    cloudy = elist.entities.find_by_solution("{'weather': 'cloudy'}")
    cloudy_terms = [{ 'term' => 'cloudy', 'locale' => '*' }, { 'term' => 'غائم', 'locale' => 'ar' }]
    assert_equal cloudy_terms, cloudy.terms
    assert_equal false, cloudy.auto_solution_enabled
    assert_equal "{'weather': 'cloudy'}", cloudy.solution
    assert_equal 2, cloudy.position
    assert_equal 6, Entity.all.count
    assert_equal ["*", "en", "fr", "es", "ar"], elist.agent.locales
  end


  test 'Entities count column is updated after import' do
    elist = entities_lists(:weather_conditions)

    io = StringIO.new
    io << "Terms,Auto solution,Solution\n"
    io << "snow,false,\"{'w': 'snow'}\"\n"
    io << "cloudy|غائم:ar,False,\"{'weather': 'cloudy'}\"\n"
    io << "\n"

    entities_import = get_entities_import(elist, io)
    assert_equal 2, elist.entities.count
    assert_equal 2, elist.entities_count
    assert entities_import.save
    assert_equal 2, entities_import.proceed

    elist.reload
    assert_equal 4, elist.entities.count
    assert_equal 4, elist.entities_count
  end


  test 'Import entities missing header' do
    io = StringIO.new
    io << "snow,false,\"{'w': 'snow'}\"\n"
    io << "cloudy|nuageux:fr,True,\"{'weather': 'cloudy'}\"\n"
    io << "\n"

    elist = entities_lists(:weather_conditions)
    entities_import = get_entities_import(elist, io)

    assert_equal 2, elist.entities.count
    assert entities_import.save
    assert_equal 0, entities_import.proceed
    assert_equal 2, elist.entities.count
    assert_equal ['Bad CSV format: Missing header'], entities_import.errors[:file]
  end


  test 'Import entities empty terms' do
    io = StringIO.new
    io << "Terms,Auto solution,Solution\n"
    io << "\"\",false,hail\n"
    io << "cloudy|nuageux:fr,True,\"{'weather': 'cloudy'}\"\n"

    elist = entities_lists(:weather_conditions)
    entities_import = get_entities_import(elist, io)

    assert_equal 2, elist.entities.count
    assert entities_import.save
    assert_equal 0, entities_import.proceed
    assert_equal 2, elist.entities.count
    assert_equal ["Validation failed: Terms can't be blank"], entities_import.errors[:file]
  end


  test 'Import entities unexpected auto solution' do
    io = StringIO.new
    io << "Terms,Auto solution,Solution\n"
    io << "snow,blablabla,snow\n"
    io << "cloudy|nuageux:fr,True,\"{'weather': 'cloudy'}\"\n"
    elist = entities_lists(:weather_conditions)
    entities_import = get_entities_import(elist, io, 'replace')

    assert_equal 2, elist.entities.count
    assert entities_import.save
    assert_equal 0, entities_import.proceed
    assert_equal 2, elist.entities.count
    assert_equal ["Validation failed: Auto solution must be true or false"], entities_import.errors[:file]
  end


  test 'Import entities autosolution true with solution' do
    io = StringIO.new
    io << "terms,auto solution,solution\n"
    io << "cloudy,True,\"{'weather': 'cloudy'}\"\n"
    elist = entities_lists(:weather_conditions)
    entities_import = get_entities_import(elist, io, 'replace')

    assert entities_import.save
    assert_equal 1, entities_import.proceed
    elist.entities.reload
    cloudy = elist.entities.first
    assert_equal [{ 'term' => 'cloudy', 'locale' => '*' }], cloudy.terms
    assert_equal true, cloudy.auto_solution_enabled
    assert_equal 'cloudy', cloudy.solution
  end


  test 'Import entities autosolution true without solution' do
    io = StringIO.new
    io << "terms,auto solution,solution\n"
    io << "cloudy,True\n"
    elist = entities_lists(:weather_conditions)
    entities_import = get_entities_import(elist, io, 'replace')

    assert entities_import.save
    assert_equal 1, entities_import.proceed
    elist.entities.reload
    cloudy = elist.entities.first
    assert_equal [{ 'term' => 'cloudy', 'locale' => '*' }], cloudy.terms
    assert_equal true, cloudy.auto_solution_enabled
    assert_equal 'cloudy', cloudy.solution
  end


  test 'Import entities autosolution false with solution' do
    io = StringIO.new
    io << "terms,auto solution,solution\n"
    io << "cloudy,false,\"{'weather': 'cloudy'}\"\n"
    elist = entities_lists(:weather_conditions)
    entities_import = get_entities_import(elist, io, 'replace')

    assert entities_import.save
    assert_equal 1, entities_import.proceed
    elist.entities.reload
    cloudy = elist.entities.first
    assert_equal [{ 'term' => 'cloudy', 'locale' => '*' }], cloudy.terms
    assert_equal false, cloudy.auto_solution_enabled
    assert_equal "{'weather': 'cloudy'}", cloudy.solution
  end


  test 'Import entities autosolution false without solution' do
    io = StringIO.new
    io << "terms,auto solution,solution\n"
    io << "cloudy,false\n"
    elist = entities_lists(:weather_conditions)
    entities_import = get_entities_import(elist, io, 'replace')

    assert entities_import.save
    assert_equal 0, entities_import.proceed
    assert_equal ['Bad CSV format: Missing column in line 1'], entities_import.errors[:file]
  end


  test 'Import entities wrong separator' do
    io = StringIO.new
    io << "Terms,Auto solution,Solution\n"
    io << "snow;false;snow\n"
    elist = entities_lists(:weather_conditions)
    entities_import = get_entities_import(elist, io)

    assert_equal 2, elist.entities.count
    assert entities_import.save
    assert_equal 0, entities_import.proceed
    assert_equal 2, elist.entities.count
    assert_equal ['Bad CSV format: Missing column in line 1'], entities_import.errors[:file]
  end


  test 'Import and replace entities' do
    io = StringIO.new
    io << "Terms,Auto solution,Solution\n"
    io << "snow,false,\"{'w': 'snow'}\"\n"
    io << "\n"
    elist = entities_lists(:weather_conditions)
    entities_import = get_entities_import(elist, io, 'replace')

    assert_equal 2, elist.entities.count
    assert entities_import.save
    assert_equal 1, entities_import.proceed
    assert_equal 1, elist.entities.count

    snow = elist.entities.find_by_solution("{'w': 'snow'}")
    snow_terms = [{ 'term' => 'snow', 'locale' => '*' }]
    assert_equal snow_terms, snow.terms
    assert_equal false, snow.auto_solution_enabled
    assert_equal "{'w': 'snow'}", snow.solution
    assert_equal 3, Entity.all.count
  end


  test 'Import entities with non existing locale' do
    elist = entities_lists(:weather_conditions)
    assert_equal ["*", "en", "fr", "es"], elist.agent.locales

    io = StringIO.new
    io << "Terms,Auto solution,Solution\n"
    io << "snow,false,\"{'w': 'snow'}\"\n"
    io << "cloudy:en|nuageuse:rf,False,\"{'weather': 'cloudy'}\"\n"
    io << "\n"

    entities_import = get_entities_import(elist, io)

    assert_equal 2, elist.entities.count
    assert entities_import.save
    assert_equal 0, entities_import.proceed
    assert_equal 2, elist.entities.count
    assert_equal ["Validation failed: Terms uses an unauthorized locale 'rf' for this agent"], entities_import.errors[:file]
  end


  private

    def get_entities_import(elist, io, mode = 'append')
      io.rewind
      File.open('temp.csv', 'w+') do |f|
        IO.copy_stream(io, f)
      end
      EntitiesImport.new(
        file:File.open('temp.csv'),
        mode: mode,
        entities_list: elist,
        user: users(:admin)
      )
    end
end
