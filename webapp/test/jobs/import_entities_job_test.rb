require 'test_helper'
require 'model_test_helper'

class ImportEntitiesJobTest < ActiveJob::TestCase

  setup do
    create_entities_import_fixtures
  end


  test 'Job is enqueued for valid file' do
    user = users(:admin)
    entities_list = entities_lists(:weather_conditions)

    assert_enqueued_with(job: ImportEntitiesJob) do
      entities_list.from_csv(@weather_conditions_import, user)
    end
  end


  test 'Job is not enqueued for invalid file' do
    user = users(:admin)
    entities_list = entities_lists(:weather_conditions)
    @weather_conditions_import.file = nil

    assert_no_enqueued_jobs do
      entities_list.from_csv(@weather_conditions_import, user)
    end
  end


  test 'Successful job execution' do
    user = users(:admin)
    entities_list = entities_lists(:weather_conditions)
    ImportEntitiesJob.perform_now(@weather_conditions_import, user)
    assert_equal 3, entities_list.entities.count
    assert_equal 3, entities_list.reload.entities_count
    assert @weather_conditions_import.destroyed?
  end


  test 'Unsuccessful job execution' do
    user = users(:admin)
    entities_list = entities_lists(:weather_conditions)

    file_path = File.join(Rails.root, 'test', 'fixtures', 'files', 'weather_condition_import.csv')
    CSV.open(file_path, "wb") do |csv|
      csv << ["terms", "auto solution", "solution"]
      csv << ["cloudy", "blabla", "\"{'weather': 'cloudy'}\"\n"]
    end

    @weather_conditions_import.file = File.open(file_path)
    assert @weather_conditions_import.save

    ImportEntitiesJob.perform_now(@weather_conditions_import, user)
    assert_equal 2, entities_list.entities.count
    assert_equal 2, entities_list.reload.entities_count
    assert @weather_conditions_import.destroyed?
  end

end
