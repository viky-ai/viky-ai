require 'application_system_test_case'
require 'model_test_helper'

class AgentRegressionChecksTest < ApplicationSystemTestCase

  def setup
    create_agent_regression_check_fixtures
  end

  test 'Add new test' do
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
      assert page.has_content?('3 tests')
      assert page.has_content?('1 running, 1 success, 1 failure')

      click_button 'Add to tests suite'
      assert page.has_content?('Not run yet...')

      assert page.has_content?('4 tests')
      find('#console-footer').click
    end

    within('#console-ts') do
      sleep 0.2 # Wait Animation
      assert page.has_content?('4 tests')
      assert 4, find('ul.cts__list').all('li').count
    end
  end

  test 'Add a new test for entities list' do
    go_to_agent_show(users(:edit_on_agent_weather), agents(:weather))

    Nlp::Interpret.any_instance.stubs('proceed').returns(
      status: 200,
      body: {
        'interpretations' => [{
          'id' => entities_lists(:weather_conditions).id,
          'slug' => entities_lists(:weather_conditions).slug,
          'package' => entities_lists(:weather_conditions).agent.id,
          'score' => '1.0',
          'solution' => entities(:weather_sunny).solution
        }]
      }
    )

    within('.console') do
      fill_in 'interpret[sentence]', with: "sun"
      first('button').click
      assert page.has_content?('3 tests')
      assert page.has_content?('1 running, 1 success, 1 failure')

      click_button 'Add to tests suite'
      assert page.has_content?('Not run yet...')

      assert page.has_content?('4 tests')
      find('#console-footer').click
    end

    within('#console-ts') do
      sleep 0.2 # Wait Animation
      assert page.has_content?('4 tests')
      assert 4, find('ul.cts__list').all('li').count
    end

  end

  test 'Add new test with language and now' do
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
      all('.dropdown__trigger > .btn')[0].click
      click_link 'en'
      all('.dropdown__trigger > .btn')[2].click
      click_link 'Manual now'
      fill_in 'interpret[now]', with: "2017-12-05T15:14:01+01:00"
      first('button').click
      assert page.has_content?('3 tests')
      assert page.has_content?('1 running, 1 success, 1 failure')

      click_button 'Add to tests suite'
      assert page.has_content?('Not run yet...')

      assert page.has_content?('4 tests')
      find('#console-footer').click
    end

     within('#console-ts') do
      sleep 0.2 # Wait Animation
      assert page.has_content?('4 tests')
      assert 4, find('ul.cts__list').all('li').count
      assert page.has_link?('hello')
      click_link("hello")

     within('.cts-item__full') do
        assert page.has_content?("hello")
        assert page.has_content?("SLUG (Expected)")
        assert page.has_content?("SOLUTION (Expected)")
        assert page.has_content?('LANGUAGE')
        assert page.has_content?('en')
        assert page.has_content?('NOW')
        assert page.has_content?('2017-12-05T15:14:01.000+01:00')
      end
    end
  end

  test 'Try to add a new test but sentence is too long' do
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
      fill_in 'interpret[sentence]', with: "A "*101
      all('button').last.click
      assert page.has_content?('Adding this to the tests suite is not possible')
    end
  end


  test 'Can only add test for the first intent' do
    go_to_agent_show(users(:admin), agents(:terminator))
    Nlp::Interpret.any_instance.stubs('proceed').returns(
      status: 200,
      body: {
        interpretations: [{
          'id' => intents(:terminator_find).id,
          'slug' => intents(:terminator_find).slug,
          'package' => intents(:terminator_find).agent.id,
          'score' => '1.0',
          'solution' => interpretations(:terminator_find_sarah).solution
        }, {
          'id' => intents(:weather_forecast).id,
          'slug' => intents(:weather_forecast).slug,
          'package' => intents(:weather_forecast).agent.id,
          'score' => '1.0',
          'solution' => interpretations(:weather_forecast_tomorrow).solution
        }]
      }
    )
    within('.console') do
      fill_in 'interpret[sentence]', with: "weather terminator"
      all('button').last.click
      assert page.has_text? '2 interpretations found.'
      assert page.has_button? 'Add to tests suite'
      assert_equal 1, find_all('.c-intents button').count
      assert_equal 1, find_all('.c-intents > li:first-child button').count
    end
  end


  test 'Detect interpretation already tested' do
    go_to_agent_show(users(:admin), agents(:weather))
    Nlp::Interpret.any_instance.stubs('proceed').returns(
      status: 200,
      body: {
        interpretations: [{
          'id' => intents(:weather_forecast).id,
          'slug' => intents(:weather_forecast).slug,
          'package' => intents(:weather_forecast).agent.id,
          'score' => '1.0',
          'solution' => interpretations(:weather_forecast_tomorrow).solution
        }]
      }
    )
    within('.console') do
      fill_in 'interpret[sentence]', with: "Quel temps fera-t-il demain ?"
      all('button').last.click
      assert page.has_no_button?('Update')
      assert page.has_no_button?('Add to tests suite')

      all('.dropdown__trigger > .btn')[0].click
      click_link 'en'
      all('button').last.click
      assert page.has_button?('Add to tests suite')
      all('.dropdown__trigger > .btn')[0].click
      click_link 'Auto'
      click_button 'Manual'
      fill_in 'interpret[now]', with: '2019-02-11T01:00:00+01:00'
      all('button').last.click
      assert page.has_button?('Add to tests suite')
    end
  end


  test 'Display tests suite panel' do
    go_to_agent_show(users(:edit_on_agent_weather), agents(:weather))
    within('.console') do
      assert page.has_content?('3 tests')
      assert page.has_content?('1 running, 1 success, 1 failure')
      find('#console-footer').click
    end

    within('#console-ts') do
      assert page.has_content?('3 tests')
      assert page.has_content?('1 running, 1 success, 1 failure')
      assert 2, find('.cts__list').all('li').count
      assert page.has_content?("Quel temps fera-t-il demain ?")
      assert page.has_content?("What's the weather like in London?")

      within('ul.cts__list') do
        click_link("Quel temps fera-t-il demain ?")
        assert page.has_content?("Quel temps fera-t-il demain ?")
        assert page.has_content?("SLUG (Expected)")
        assert page.has_content?("SOLUTION (Expected)")
      end
    end
  end


  test 'Edit and delete button should be disabled for running tests' do
    go_to_agent_show(users(:edit_on_agent_weather), agents(:weather))

    within('.console') do
      assert page.has_content?('3 tests')
      assert page.has_content?('1 running, 1 success, 1 failure')
      find('#console-footer').click
      sleep 0.2 # Wait Animation
    end

    within('#console-ts') do
      assert page.has_content?('3 tests')
      assert page.has_content?('1 running, 1 success, 1 failure')
      click_link("Quel temps fera-t-il demain ?")
    end

    within('.cts-item__full') do
      assert page.has_button?('Delete', disabled: true)
      assert page.has_button?('Send', disabled: true)
    end
  end


  test 'Delete test - success' do
    go_to_agent_show(users(:edit_on_agent_weather), agents(:weather))

    within('.console') do
      assert page.has_content?('3 tests')
      assert page.has_content?('1 running, 1 success, 1 failure') # TODO
      find('#console-footer').click
      sleep 0.2 # Wait Animation
    end

    within('#console-ts') do
      assert page.has_content?('3 tests')
      assert page.has_content?('1 running, 1 success, 1 failure') # TODO
      click_link("What's the weather like in London?")
      assert page.has_text?('Delete')
      click_button('Delete')
    end

    within('.cts-item-delete') do
      assert page.has_content?('Are you sure?')
      assert page.has_button?('Yes, delete')
      assert page.has_link?('No, cancel')
      click_button('Yes, delete')
    end

    within('#console-ts') do
      assert page.has_content?('2 tests')
      assert page.has_content?('1 running, 1 success, 0 failure')
      assert 1, find('ul.cts__list').all('li').count
      assert page.has_no_content?("What's the weather like in London ?")
    end
  end


  test 'Send test' do
    go_to_agent_show(users(:edit_on_agent_weather), agents(:weather))

    Nlp::Interpret.any_instance.stubs('proceed').returns(
      status: 200,
      body: {
        interpretations: [{
          'id' => intents(:weather_forecast).id,
          'slug' => intents(:weather_forecast).slug,
          'package' => intents(:weather_forecast).agent.id,
          'score' => '1.0',
          'solution' => interpretations(:weather_question_like).solution
        }]
      }
    )

    within('#console') do
      assert page.has_content?('3 tests')
      assert page.has_content?('1 running, 1 success, 1 failure')
      find('#console-footer').click
      sleep 0.2 # Wait Animation
    end

    within('#console-ts') do
      assert page.has_content?('3 tests')
      assert page.has_content?('1 running, 1 success, 1 failure')
      click_link("What's the weather like in London?")
    end

    within('.cts-item__full') do
      assert page.has_content?("What's the weather like in London?")
      assert page.has_button?('Send')
      click_button('Send')
    end

    within('#js-console-form') do
      assert page.has_content?('en')
      assert first('button[data-trigger-event="console-select-now-type-manual"]').matches_css?(".btn--primary")
    end

    within('#console-explain') do
      assert page.has_content?('1 interpretation found.')
      assert page.has_content?('Failed')
      assert page.has_button?('Update')
      click_button('Update')
      assert page.has_content?('Not run yet...')
    end

    assert page.has_content?('3 tests')
  end


  test 'Regression check fail when different slugs but same solutions' do
    @regression_weather_condition.got = {
      'root_type' => 'intent',
      'package'   => intents(:weather_question).agent.id,
      'id'        => intents(:weather_question).id,
      'slug'      => intents(:weather_question).slug,
      'solution'  => interpretations(:weather_question_like).solution.to_json.to_s
    }
    @regression_weather_condition.state = 'failure'
    assert @regression_weather_condition.save

    go_to_agent_show(users(:edit_on_agent_weather), agents(:weather))

    within('.console') do
      assert page.has_content?('3 tests')
      find('#console-footer').click
      sleep 0.2 # Wait Animation
    end

    within('#console-ts') do
      assert page.has_content?('3 tests')
      click_link('Sun today')
    end

    within('.cts-item__full') do
      assert page.has_content?('Sun today')
      assert find_all('.cts-item__full__detail__value').one? { |d| d.text == "\"sun\"" }
    end
  end


  test 'Failed regression check with got referring to an entities list' do
    @regression_weather_question.got = {
      'root_type' => 'entities_list',
      'package'   => entities_lists(:weather_conditions).agent.id,
      'id'        => entities_lists(:weather_conditions).id,
      'slug'      => entities_lists(:weather_conditions).slug,
      'solution'  => entities(:weather_raining).solution.to_json.to_s
    }
    @regression_weather_question.state = 'failure'
    assert @regression_weather_question.save

    go_to_agent_show(users(:edit_on_agent_weather), agents(:weather))

    within('.console') do
      assert page.has_content?('3 tests')
      find('#console-footer').click
      sleep 0.2 # Wait Animation
    end

    within('#console-ts') do
      assert page.has_content?('3 tests')
      assert page.has_content?('1 running, 1 success, 1 failure')
      click_link("What's the weather like in London?")
    end

    within('.cts-item__full') do
      assert page.has_content?("What's the weather like in London?")
      assert page.has_content?(intents(:weather_question).slug)
      assert page.has_content?(entities_lists(:weather_conditions).slug)
    end
  end
end
