require "application_system_test_case"
require "model_test_helper"

class AgentRegressionChecksTest < ApplicationSystemTestCase

  def setup
    super
    create_agent_regression_check_fixtures
  end

  # We isolate websocket communication between test in order
  # to activate parallel testing
  def isolate_actioncable_and_go_to_ui
    user = users(:edit_on_agent_weather)
    agent = AgentDuplicator.new(agents(:weather), user).duplicate
    user_go_to_agent_show(users(:edit_on_agent_weather), agent)
  end


  test "Add new test" do
    isolate_actioncable_and_go_to_ui

    Nlp::Interpret.any_instance.stubs("send_nlp_request").returns(
      status: 200,
      body: {
        "interpretations" => [
          {
            "id" => intents(:weather_forecast).id,
            "slug" => intents(:weather_forecast).slug,
            "package" => intents(:weather_forecast).agent.id,
            "score" => "1.0",
            "solution" => formulations(:weather_forecast_tomorrow).solution
          }
        ]
      }
    )

    within(".console") do
      fill_in "interpret[sentence]", with: "hello"
      click_button "console-send-sentence"
      assert has_text?("3 tests")
      assert has_text?("1 running, 1 success, 1 failure")

      click_button "Add to tests suite"
      assert has_text?("Not run yet...")

      assert has_text?("4 tests")
      find("#console-footer").click
    end

    within("#console-ts") do
      sleep 0.2 # Wait Animation
      assert has_text?("4 tests")
      assert 4, find("ul.cts__list").all("li").count
    end
  end


  test "Add a new test for entities list" do
    isolate_actioncable_and_go_to_ui

    Nlp::Interpret.any_instance.stubs("send_nlp_request").returns(
      status: 200,
      body: {
        "interpretations" => [{
          "id" => entities_lists(:weather_conditions).id,
          "slug" => entities_lists(:weather_conditions).slug,
          "package" => entities_lists(:weather_conditions).agent.id,
          "score" => "1.0",
          "solution" => entities(:weather_sunny).solution
        }]
      }
    )

    within(".console") do
      assert has_text?("3 tests")
      assert has_text?("1 running, 1 success, 1 failure")

      fill_in "interpret[sentence]", with: "sun"
      click_button "console-send-sentence"

      click_button "Add to tests suite"
      assert has_text?("Not run yet...")

      assert has_text?("4 tests")
      find("#console-footer").click
    end

    within("#console-ts") do
      sleep 0.2 # Wait Animation
      assert has_text?("4 tests")
      assert has_css?("ul.cts__list li", count: 4)
    end
  end


  test "Add new test with language, now and spellchecking" do
    isolate_actioncable_and_go_to_ui

    Nlp::Interpret.any_instance.stubs("send_nlp_request").returns(
      status: 200,
      body: {
        "interpretations" => [
          {
            "id" => intents(:weather_forecast).id,
            "slug" => intents(:weather_forecast).slug,
            "package" => intents(:weather_forecast).agent.id,
            "score" => "1.0",
            "solution" => formulations(:weather_forecast_tomorrow).solution
          }
        ]
      }
    )

    within(".console") do
      assert has_text?("3 tests")
      assert has_text?("1 running, 1 success, 1 failure")

      all(".dropdown__trigger > .btn")[0].click
      click_link "en"
      all(".dropdown__trigger > .btn")[1].click
      click_link "Medium"
      click_button "Manual"
      fill_in "interpret[now]", with: "2017-12-05T15:14:01+01:00"
      fill_in "interpret[sentence]", with: "hello"
      click_button "console-send-sentence"

      click_button "Add to tests suite"
      assert has_text?("4 tests")
      find("#console-footer").click
    end

    within("#console-ts") do
      sleep 0.2 # Wait Animation
      assert has_text?("4 tests")
      assert 4, find("ul.cts__list").all("li").count
      assert has_link?("hello")
      click_link("hello")

      within(".cts-item__full") do
        assert has_text?("hello")
        assert has_text?("SLUG (Expected)")
        assert has_text?("SOLUTION (Expected)")
        assert has_text?("LANGUAGE")
        assert has_text?("en")
        assert has_text?("SPELLCHECKING")
        assert has_text?("Medium")
        assert has_text?("NOW")
        assert has_text?("2017-12-05T15:14:01.000+01:00")
      end
    end
  end


  test "Try to add a new test but sentence is too long" do
    isolate_actioncable_and_go_to_ui

    Nlp::Interpret.any_instance.stubs("send_nlp_request").returns(
      status: 200,
      body: {
        "interpretations" => [
          {
            "id" => intents(:weather_forecast).id,
            "slug" => intents(:weather_forecast).slug,
            "package" => intents(:weather_forecast).agent.id,
            "score" => "1.0",
            "solution" => formulations(:weather_forecast_tomorrow).solution
          }
        ]
      }
    )
    within(".console") do
      fill_in "interpret[sentence]", with: "A "*101
      click_button "console-send-sentence"
      assert has_text?("Adding this to the tests suite is not possible")
    end
  end


  test "Can only add test for the first intent" do
    isolate_actioncable_and_go_to_ui

    Nlp::Interpret.any_instance.stubs("send_nlp_request").returns(
      status: 200,
      body: {
        "interpretations" => [{
          "id" => intents(:terminator_find).id,
          "slug" => intents(:terminator_find).slug,
          "package" => intents(:terminator_find).agent.id,
          "score" => "1.0",
          "solution" => formulations(:terminator_find_sarah).solution
        }, {
          "id" => intents(:weather_forecast).id,
          "slug" => intents(:weather_forecast).slug,
          "package" => intents(:weather_forecast).agent.id,
          "score" => "1.0",
          "solution" => formulations(:weather_forecast_tomorrow).solution
        }]
      }
    )
    within(".console") do
      fill_in "interpret[sentence]", with: "weather terminator"
      click_button "console-send-sentence"
      assert has_text? "2 interpretations found."
      assert has_button? "Add to tests suite"
      assert has_css?(".c-intents button", count: 1)
      assert has_css?(".c-intents > li:first-child button", count: 1)
    end
  end


  test "Detect formulation already tested" do
    isolate_actioncable_and_go_to_ui

    Nlp::Interpret.any_instance.stubs("send_nlp_request").returns(
      status: 200,
      body: {
        "interpretations" => [{
          "id" => intents(:weather_forecast).id,
          "slug" => intents(:weather_forecast).slug,
          "package" => intents(:weather_forecast).agent.id,
          "score" => "1.0",
          "solution" => formulations(:weather_forecast_tomorrow).solution
        }]
      }
    )
    within(".console") do
      fill_in "interpret[sentence]", with: "Quel temps fera-t-il demain ?"
      click_button "console-send-sentence"
      assert has_no_button?("Update")
      assert has_no_button?("Add to tests suite")

      all(".dropdown__trigger > .btn")[0].click
      click_link "en"
      click_button "console-send-sentence"
      assert has_button?("Add to tests suite")
      all(".dropdown__trigger > .btn")[0].click
      click_link "Auto"
      click_button "Manual"
      fill_in "interpret[now]", with: "2019-02-11T01:00:00+01:00"
      click_button "console-send-sentence"
      assert has_button?("Add to tests suite")
    end
  end


  test "Display tests suite panel" do
    isolate_actioncable_and_go_to_ui

    within(".console") do
      assert has_text?("3 tests")
      assert has_text?("1 running, 1 success, 1 failure")
      find("#console-footer").click
    end

    within("#console-ts") do
      assert has_text?("3 tests")
      assert has_text?("1 running, 1 success, 1 failure")
      assert 2, find(".cts__list").all("li").count
      assert has_text?("Quel temps fera-t-il demain ?")
      assert has_text?("What's the weather like in London?")

      within("ul.cts__list") do
        click_link("Quel temps fera-t-il demain ?")
        assert has_text?("Quel temps fera-t-il demain ?")
        assert has_text?("SLUG (Expected)")
        assert has_text?("SOLUTION (Expected)")
      end
    end
  end


  test "Edit and delete button should be disabled for running tests" do
    isolate_actioncable_and_go_to_ui

    within(".console") do
      assert has_text?("3 tests")
      assert has_text?("1 running, 1 success, 1 failure")
      find("#console-footer").click
      sleep 0.2 # Wait Animation
    end

    within("#console-ts") do
      assert has_text?("3 tests")
      assert has_text?("1 running, 1 success, 1 failure")
      click_link("Quel temps fera-t-il demain ?")
    end

    within(".cts-item__full") do
      assert has_button?("Delete", disabled: true)
      assert has_button?("Send", disabled: true)
    end
  end


  test "Delete test - success" do
    isolate_actioncable_and_go_to_ui

    within(".console") do
      assert has_text?("3 tests")
      assert has_text?("1 running, 1 success, 1 failure")
      find("#console-footer").click
      sleep 0.2 # Wait Animation
    end

    within("#console-ts") do
      assert has_text?("3 tests")
      assert has_text?("1 running, 1 success, 1 failure")
      click_link("What's the weather like in London?")
      assert has_text?("Delete")
      click_button("Delete")
    end

    within(".cts-item-delete") do
      assert has_text?("Are you sure?")
      assert has_button?("Yes, delete")
      assert has_link?("No, cancel")
      click_button("Yes, delete")
    end

    within("#console-ts") do
      assert has_text?("2 tests")
      assert has_text?("1 running, 1 success, 0 failure")
      assert 1, find("ul.cts__list").all("li").count
      assert has_no_text?("What's the weather like in London ?")
    end
  end


  test "Send test" do
    isolate_actioncable_and_go_to_ui

    Nlp::Interpret.any_instance.stubs("send_nlp_request").returns(
      status: 200,
      body: {
        "interpretations" => [{
          "id" => intents(:weather_forecast).id,
          "slug" => intents(:weather_forecast).slug,
          "package" => intents(:weather_forecast).agent.id,
          "score" => "1.0",
          "solution" => formulations(:weather_question_like).solution
        }]
      }
    )

    within("#console") do
      assert has_text?("3 tests")
      assert has_text?("1 running, 1 success, 1 failure")
      find("#console-footer").click
      sleep 0.2 # Wait Animation
    end

    within("#console-ts") do
      assert has_text?("3 tests")
      assert has_text?("1 running, 1 success, 1 failure")
      click_link("What's the weather like in London?")
    end

    within(".cts-item__full") do
      assert has_text?("What's the weather like in London?")
      assert has_text?("Send")
      click_button("Send")
    end

    within("#js-console-form") do
      assert has_text?("en")
      assert has_text?("High")
      assert first('button[data-trigger-event="console-select-now-type-manual"]').matches_css?(".btn--primary")
    end

    within("#console-explain") do
      assert has_text?("1 interpretation found.")
      assert has_text?("Failed")
      assert has_button?("Update")
      click_button("Update")
      assert has_text?("Not run yet...")
    end

    assert has_text?("3 tests")
  end


  test "Regression check fail when different slugs but same solutions" do
    @regression_weather_condition.got = {
      "root_type" => "intent",
      "package"   => intents(:weather_question).agent.id,
      "id"        => intents(:weather_question).id,
      "slug"      => intents(:weather_question).slug,
      "solution"  => formulations(:weather_question_like).solution.to_json.to_s
    }
    @regression_weather_condition.state = "failure"
    assert @regression_weather_condition.save

    isolate_actioncable_and_go_to_ui

    within(".console") do
      assert has_text?("3 tests")
      find("#console-footer").click
      sleep 0.2 # Wait Animation
    end

    within("#console-ts") do
      assert has_text?("3 tests")
      click_link("Sun today")
    end

    within(".cts-item__full") do
      assert has_text?("Sun today")
      assert find_all(".cts-item__full__detail__value").one? { |d| d.text == "\"sun\"" }
    end
  end


  test "Failed regression check with got referring to an entities list" do
    @regression_weather_question.got = {
      "root_type" => "entities_list",
      "package"   => entities_lists(:weather_conditions).agent.id,
      "id"        => entities_lists(:weather_conditions).id,
      "slug"      => entities_lists(:weather_conditions).slug,
      "solution"  => entities(:weather_raining).solution.to_json.to_s
    }
    @regression_weather_question.state = "failure"
    assert @regression_weather_question.save

    isolate_actioncable_and_go_to_ui

    within(".console") do
      assert has_text?("3 tests")
      find("#console-footer").click
      sleep 0.2 # Wait Animation
    end

    within("#console-ts") do
      assert has_text?("3 tests")
      assert has_text?("1 running, 1 success, 1 failure")
      click_link("What's the weather like in London?")
    end

    within(".cts-item__full") do
      assert has_text?("What's the weather like in London?")
      assert has_text?("edit_on_agent_weather/weather-copy/interpretations/weather_question")
      assert has_text?(entities_lists(:weather_conditions).slug)
    end
  end
end
