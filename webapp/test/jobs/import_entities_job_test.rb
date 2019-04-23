require 'test_helper'
require 'model_test_helper'

class ImportEntitiesJobTest < ActiveJob::TestCase

  setup do
    create_entities_import_fixtures
  end


  test 'Successful job execution' do
    user = users(:admin)
    entities_list = entities_lists(:weather_conditions)
    ImportEntitiesJob.perform_now(@weather_conditions_import)
    assert_equal 3, entities_list.entities.count
    assert_equal 3, entities_list.reload.entities_count
    assert_nil @weather_conditions_import.file
    assert_equal "success", @weather_conditions_import.status
  end


  test 'Unsuccessful job execution' do
    user = users(:admin)
    entities_list = entities_lists(:weather_conditions)

    file_path = File.join(Rails.root, 'tmp', 'weather_condition_import.csv')
    CSV.open(file_path, "wb") do |csv|
      csv << ["terms", "auto solution", "solution"]
      csv << ["cloudy", "blabla", "\"{'weather': 'cloudy'}\"\n"]
    end

    @weather_conditions_import.file = File.open(file_path)
    assert @weather_conditions_import.save

    ImportEntitiesJob.perform_now(@weather_conditions_import)
    assert_equal 2, entities_list.entities.count
    assert_equal 2, entities_list.reload.entities_count
    assert_nil @weather_conditions_import.file
    assert_equal "failure", @weather_conditions_import.status
    assert_equal 0, @weather_conditions_import.duration
  end


  def create_entities_import_fixtures
    file_path = File.join(Rails.root, 'tmp', 'weather_condition_import.csv')
    CSV.open(file_path, "wb") do |csv|
      csv << ["terms", "auto solution", "solution"]
      csv << ["cloudy", "True", "\"{'weather': 'cloudy'}\"\n"]
    end

    @weather_conditions_import = EntitiesImport.new({
      file: File.open(file_path),
      mode: 'append',
      entities_list: entities_lists(:weather_conditions),
      user: users(:admin)
    })
    @weather_conditions_import.save!
  end

end
