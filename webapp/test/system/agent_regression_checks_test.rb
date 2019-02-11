require 'application_system_test_case'
require 'model_test_helper'

class AgentRegressionChecksTest < ApplicationSystemTestCase

  def setup
    create_agent_regression_check_fixtures
  end

  test 'Add new regression test' do
    go_to_agent_show(users(:edit_on_agent_weather), agents(:weather))
    Nlp::Interpret.any_instance.stubs('proceed').returns(
      status: 200,
      body: {
        interpretations: [
          {
            'id' => intents(:weather_forecast).id,
            'slug' => intents(:weather_forecast).slug,
            'package' => intents(:weather_forecast).agent.id,
            'score' => '1.0',
            'solution' => interpretations(:weather_forecast_tomorrow).solution
          }
        ]
      }
    )
    within('.console') do
      fill_in 'interpret[sentence]', with: "hello"
      first('button').click
      assert page.has_content?('2 tests, 1 failure')
      click_button 'Add to tests suite'
      assert page.has_content?('3 tests, 1 failure')
    end
  end

  test 'Can only add the first intent' do
    go_to_agent_show(users(:admin), agents(:terminator))
    Nlp::Interpret.any_instance.stubs('proceed').returns(
      {
        status: 200,
        body: {
          interpretations: [
            {
              "id" => intents(:terminator_find).id,
              "slug" => "admin/terminator/terminator_find",
              "name" => "terminator_find",
              "score" => 1.0
            },
            {
              "id" => intents(:weather_forecast).id,
              "slug" => "admin/weather/weather_forecast",
              "name" => "weather_forecast",
              "score" => 1.0
            }
          ]
        }
      }
    )
    within('.console') do
      fill_in 'interpret[sentence]', with: "weather terminator"
      first('button').click
      assert page.has_text? '2 interpretations found.'
      assert page.has_button? 'Add to tests suite'
      assert_equal 1, find('.c-intents').all('input.btn').count
      assert_equal 1, find('.c-intents > li:first-child').all('input.btn').count
    end
  end

  test 'Detect sentence already tested' do
    go_to_agent_show(users(:admin), agents(:weather))
    Nlp::Interpret.any_instance.stubs('proceed').returns(
      status: 200,
      body: {
        interpretations: [{
          "id" => intents(:weather_forecast).id,
          "slug" => "admin/weather/weather_forecast",
          "name" => "weather_forecast",
          "score" => 1.0
        }]
      }
    )
    within('.console') do
      fill_in 'interpret[sentence]', with: "Quel temps fera-t-il demain ?"
      first('button').click
      assert page.has_text?('Update test')
      all('.dropdown__trigger > .btn')[0].click
      click_link 'en'
      first('button').click
      assert page.has_button?('Add to tests suite')
      all('.dropdown__trigger > .btn')[0].click
      click_link 'Language: Auto'
      all('.dropdown__trigger > .btn')[2].click
      click_link 'Manual now'
      fill_in 'interpret[now]', with: '2019-02-11T01:00:00+01:00'
      first('button').click
      assert page.has_button?('Add to tests suite')
    end
  end

  test 'Regression test panel' do
    go_to_agent_show(users(:edit_on_agent_weather), agents(:weather))
    within('.console') do
      assert page.has_content?('2 tests, 1 failure')
      find('#console-footer').click
    end

    within('#console-ts') do
      assert page.has_content?('2 tests, 1 failure')
      assert 2, find('.cts__list').all('li').count
      assert page.has_content?("Quel temps fera-t-il demain ?")
      assert page.has_content?("What's the weather like in London ?")

      within('ul.cts__list') do
        click_link("Quel temps fera-t-il demain ?")
        assert page.has_content?("Quel temps fera-t-il demain ?")
        assert page.has_content?("EXPECTED & GOT")
      end
    end
  end

  test 'Regression test panel - Add new test' do
    go_to_agent_show(users(:edit_on_agent_weather), agents(:weather))
    within('.console') do
      fill_in 'interpret[sentence]', with: "hello"
      first('button').click
      Nlp::Interpret.any_instance.stubs('proceed').returns(
        status: 200,
        body: {
          interpretations: [
            {
              'id' => intents(:weather_forecast).id,
              'slug' => intents(:weather_forecast).slug,
              'package' => intents(:weather_forecast).agent.id,
              'score' => '1.0',
              'solution' => interpretations(:weather_forecast_tomorrow).solution
            }
          ]
        }
      )
      assert page.has_content?('2 tests, 1 failure')
      click_button 'Add to tests suite'
      assert page.has_content?('3 tests, 1 failure')
      find('#console-footer').click
    end

    within('#console-ts') do
      assert page.has_content?('3 tests, 1 failure')
      assert 3, find('ul.cts__list').all('li').count
      assert page.has_content?("hello")
    end
  end

  test 'Delete test - failure' do
    go_to_agent_show(users(:edit_on_agent_weather), agents(:weather))

    test_to_delete = @regression_weather_question
    @regression_weather_question.state = 'running'
    @regression_weather_question.save

    within('.console') do
      assert page.has_content?('2 tests, 1 failure')
      find('#console-footer').click
    end

    within('#console-ts') do
      assert page.has_content?('2 tests, 1 failure')
      click_link("Quel temps fera-t-il demain ?")
      assert page.has_content?('Delete')
      click_link 'Delete'
      assert page.has_content?('Are you sure?')
    end

    within('.cts-item-delete') do
      click_link 'Delete'
    end
    assert page.has_content?('2 tests, 1 failure')
    assert 2, find('ul.cts__list').all('li').count
  end

  test 'Delete test - success' do
    go_to_agent_show(users(:edit_on_agent_weather), agents(:weather))
    within('.console') do
      assert page.has_content?('2 tests, 1 failure')
      find('#console-footer').click
    end

    within('#console-ts') do
      assert page.has_content?('2 tests, 1 failure')
      click_link("Quel temps fera-t-il demain ?")
      assert page.has_content?('Delete')
      click_link 'Delete'
      assert page.has_content?('Are you sure?')
    end

    within('.cts-item-delete') do
      click_link 'Delete'
    end
    assert page.has_content?('1 test, 1 failure')
    assert 1, find('ul.cts__list').all('li').count
  end
end
