require "application_system_test_case"

class InterpretationsTest < ApplicationSystemTestCase

  test "navigation" do
    admin_go_to_agents_index
    assert has_text?("admin/weather")
    click_link "My awesome weather bot admin/weather"
    assert has_link?("Interpretations")
    click_link "Interpretations"
    assert has_text?("weather_forecast")

    click_link "weather_forecast"
    assert has_text?("Interpretations / weather_forecast PUBLIC")
  end


  test "Show an interpretation with details" do
    login_as "show_on_agent_weather@viky.ai", "BimBamBoom"
    assert has_text?("admin/weather")
    click_link "My awesome weather bot admin/weather"
    assert has_link?("Interpretations")
    click_link "Interpretations"
    assert has_text?("weather_forecast")

    click_link "weather_forecast"
    assert has_text?("Interpretations / weather_forecast PUBLIC")
    assert has_text?("Not used by any interpretation")

    within(".card .tabs") do
      click_link "en"
    end
    assert has_link?("What the weather like tomorrow ?")

    within("#interpretations-list") do
      click_link "What the weather like tomorrow ?"
      assert has_text?("admin/weather/weather_question")
      assert has_link?("admin/weather/weather_question")
      assert has_no_button?("Update")
      assert has_no_link?("Delete")
      assert has_no_field?("trix-editor")
      assert has_no_field?("input[name*='aliasname']")
    end
  end


  test "Used by button in interpretation details" do
    admin_go_to_intent_show(agents(:terminator), intents(:simple_where))
    assert has_link?("Used by...")
    click_link("Used by...")
    within(".modal__main") do
      assert has_text?("Interpretations using simple_where")
      click_link("terminator_find")
    end
    assert has_text?("Interpretations / terminator_find")
  end


  test "Create an interpretation" do
    admin_go_to_intent_show(agents(:weather), intents(:weather_forecast))

    within(".card .tabs") do
      click_link "en"
    end

    assert has_link?("What the weather like tomorrow ?")
    assert_equal "1", first("#current-locale-tab-badge").text

    first("trix-editor").click.set("Good morning")
    all(".dropdown__trigger > .btn")[0].click
    click_link "Very close"
    click_button "Add"
    assert has_text?("Good morning")

    assert_equal "2", first("#current-locale-tab-badge").text
  end


  test "Create an interpretation with intent alias" do
    admin_go_to_intent_show(agents(:weather), intents(:weather_forecast))

    within(".card .tabs") do
      click_link "en"
    end

    assert has_link?("What the weather like tomorrow ?")
    assert has_css?(".interpretation-resume", count: 1)
    assert_equal "What the weather like", first(".interpretation-resume__alias-blue").text

    first("trix-editor").click.set("Salut Marcel")
    select_text_in_trix("trix-editor", 6, 12)
    within "#popup-add-tag" do
      find_link("admin/weather/interpretations/weather_question").click
    end
    within(".aliases") do
      assert has_link?("admin/weather/interpretations/weather_question")
      assert has_text?("Marcel")
    end

    click_button "Add"

    assert has_text?("Salut Marcel")
    assert_equal 2, all(".interpretation-resume").count
    assert_equal "Marcel", first(".interpretation-resume__alias-blue").text

    first("trix-editor").click.set("Il fait beau ?")
    select_text_in_trix("trix-editor", 9, 12)
    within "#popup-add-tag" do
      find_link("admin/weather/interpretations/weather_question").click
    end
    within("#interpretations-form .aliases") do
      click_link "admin/weather/interpretations/weather_question"
    end
    assert has_text?("Interpretations / weather_question PUBLIC")
  end


  test "Create an interpretation with entities_list alias" do
    admin_go_to_intent_show(agents(:weather), intents(:weather_forecast))

    within(".card .tabs") do
      click_link "en"
    end

    assert has_link?("What the weather like tomorrow ?")
    assert has_css?(".interpretation-resume", count: 1)
    assert_equal "What the weather like", first(".interpretation-resume__alias-blue").text

    first("trix-editor").click.set("Y a-t-il du soleil ?")
    select_text_in_trix("trix-editor", 12, 18)
    within "#popup-add-tag" do
      find_link("admin/weather/entities_lists/weather_conditions").click
    end

    within(".aliases") do
      assert has_link?("admin/weather/entities_lists/weather_conditions")
      assert has_text?("soleil")
    end

    click_button "Add"

    assert has_text?("Y a-t-il du soleil ?")
    assert has_css?(".interpretation-resume", count: 2)
    assert_equal "soleil", first(".interpretation-resume__alias-purple").text

    within("#interpretations-list") do
      click_link "Y a-t-il du soleil ?"
    end
    within(".aliases") do
      find_link("admin/weather/entities_lists/weather_conditions").click
    end
    assert has_text?("Entities lists / weather_conditions PUBLIC")
  end


  test "Create an interpretation with numbers" do
    admin_go_to_intent_show(agents(:weather), intents(:weather_forecast))

    within(".card .tabs") do
      click_link "en"
    end

    assert has_link?("What the weather like tomorrow ?")
    assert has_css?(".interpretation-resume", count: 1)
    assert_equal "What the weather like", first(".interpretation-resume__alias-blue").text

    first("trix-editor").click.set("Le 01/01/2018 j'achète de l'Aspirine")
    select_text_in_trix("trix-editor", 3, 5)
    within "#popup-add-tag" do
      find_link("Number").click
    end

    select_text_in_trix("trix-editor", 6, 8)
    within "#popup-add-tag" do
      find_link("Number").click
    end

    select_text_in_trix("trix-editor", 9, 13)
    within "#popup-add-tag" do
      find_link("Number").click
    end

    within(".aliases") do
      inputs = all("input[name*='aliasname']")
      inputs[0].set("day")
      inputs[1].set("month")
      inputs[2].set("year")
    end

    click_button "Add"
    assert has_text?("Le 01/01/2018 j'achète de l'Aspirine")

    assert has_css?(".interpretation-resume", count: 2)
    expected = ["01", "01", "2018"]
    assert_equal expected, all(".interpretation-resume__alias-black").collect(&:text)

    click_link "Le 01/01/2018 j'achète de l'Aspirine"
    assert has_text?("Cancel")
    within(".aliases") do
      expected = ["day", "month", "year"]
      assert_equal expected, all("input[name*='aliasname']").collect(&:value)
    end
  end

  test "Create an interpretation with proximity value" do
    admin_go_to_intent_show(agents(:weather), intents(:weather_forecast))

    within(".card .tabs") do
      click_link "en"
    end

    within("#interpretations-list") do
      assert has_text?("What the weather like tomorrow ?")
      assert has_text?("Glued")
    end

    first("trix-editor").click.set("Sunny day")
    all(".dropdown__trigger > .btn")[0].click
    click_link "Glued + Punc"
    click_button "Add"

    within("#interpretations-list") do
      assert has_text?("Sunny day")
      assert has_text?("Glued + Punc")
    end
    assert_equal "2", first("#current-locale-tab-badge").text
  end


  test "Errors on interpretation creation" do
    admin_go_to_intent_show(agents(:weather), intents(:weather_forecast))

    assert_equal "0", first("#current-locale-tab-badge").text

    first("trix-editor").click.set("")
    click_button "Add"
    assert has_text?("Expression can't be blank")

    assert_equal "0", first("#current-locale-tab-badge").text
  end


  test "Errors on interpretation creation with alias" do
    admin_go_to_intent_show(agents(:weather), intents(:weather_forecast))

    within(".card .tabs") do
      click_link "en"
    end

    assert has_link?("What the weather like tomorrow ?")
    assert has_css?(".interpretation-resume", count: 1)
    assert_equal "What the weather like", first(".interpretation-resume__alias-blue").text

    first("trix-editor").click.set("Salut Marcel")
    select_text_in_trix("trix-editor", 6, 12)
    within "#popup-add-tag" do
      find_link("admin/weather/interpretations/weather_question").click
    end
    within(".aliases") do
      assert has_text?("admin/weather/interpretations/weather_question")
      assert has_text?("Marcel")
      first("input").set("")
    end

    click_button "Add"
    assert has_text?("Parameter name can't be blank")
  end


  test "Update an interpretation (simple)" do
    admin_go_to_intent_show(agents(:weather), intents(:weather_forecast))

    within(".card .tabs") do
      click_link "fr"
    end

    assert has_link?("Quel temps fera-t-il demain ?")
    assert_equal "1", first("#current-locale-tab-badge").text

    within("#interpretations-list") do
      click_link "Quel temps fera-t-il demain ?"
      assert has_text?("Cancel")
      first("trix-editor").click.set("Salut à tous")
      check("interpretation[keep_order]")
      uncheck("interpretation[auto_solution_enabled]")
      fill_in_editor_field "10"
      click_button "Update"
    end

    assert has_link?("Salut à tous")
    assert_equal "1", first("#current-locale-tab-badge").text
  end


  test "Update an interpretation (add alias)" do
    admin_go_to_intent_show(agents(:weather), intents(:weather_forecast))

    within(".card .tabs") do
      click_link "en"
    end
    assert has_link?("What the weather like tomorrow ?")

    within("#interpretations-list") do
      click_link "What the weather like tomorrow ?"
      assert has_text?("Cancel")
      select_text_in_trix("#interpretations-list trix-editor", 31, 32)
    end

    within("#popup-add-tag") do
      find_link("admin/weather/interpretations/weather_question").click
    end

    within("#interpretations-list") do
      within(".aliases") do
        assert has_text?("tomorrow")
        all("input[name*='is_list']").first.click
      end
      click_button "Update"
    end

    assert has_link?("What the weather like tomorrow ?")
    assert has_css?(".interpretation-resume", count: 1)
    assert_equal ["What the weather like", "?"], all(".interpretation-resume__alias-blue").collect(&:text)

    within("#interpretations-list") do
      click_link "What the weather like tomorrow ?"
      assert has_text?("Cancel")
      assert all("input[name*='is_list']")[0].checked?
      assert_not all("input[name*='is_list']")[1].checked?
    end
  end


  test "Update an interpretation (remove alias)" do
    admin_go_to_intent_show(agents(:weather), intents(:weather_forecast))

    within(".card .tabs") do
      click_link "en"
    end
    assert has_link?("What the weather like tomorrow ?")

    within("#interpretations-list") do
      click_link "What the weather like tomorrow ?"
      assert has_text?("Cancel")
      select_text_in_trix("#interpretations-list trix-editor", 0, 21)
    end
    within "#popup-remove-tag" do
      find_link("Remove annotation(s)").click
    end
    click_button "Update"

    assert has_link?("What the weather like tomorrow ?")
    assert has_css?(".interpretation-resume", count: 1)
  end

  test "Update an interpretation (change proximity)" do
    admin_go_to_intent_show(agents(:weather), intents(:weather_forecast))

    within(".card .tabs") do
      click_link "en"
    end

    assert has_link?("What the weather like tomorrow ?")
    assert has_text?("Glued")

    assert_equal "1", first("#current-locale-tab-badge").text

    within("#interpretations-list") do
      click_link "What the weather like tomorrow ?"
      assert has_text?("Cancel")
      first("trix-editor").click.set("What the weather like today ?")
      check("interpretation[keep_order]")
      all(".dropdown__trigger > .btn")[0].click
      click_link "Far"
      click_button "Update"
    end

    assert has_link?("What the weather like today ?")
    assert has_text?("Far")
    assert_equal "1", first("#current-locale-tab-badge").text
  end


  test "Remove alias from summary board" do
    admin_go_to_intent_show(agents(:weather), intents(:weather_forecast))

    within(".card .tabs") do
      click_link "en"
    end
    assert has_link?("What the weather like tomorrow ?")

    within("#interpretations-list") do
      click_link "What the weather like tomorrow ?"
      within(".aliases") do
        assert has_text?("question")
        assert has_text?("dates")
        all('a[href="#"').last.click
        all('a[href="#"').last.click
        assert has_no_text?("question")
        assert has_no_text?("dates")
      end
      question = interpretation_aliases(:weather_forecast_tomorrow_question)
      assert_no_text_selected_in_trix question.interpretation.id, question.aliasname
      click_button "Update"
    end

    assert has_link?("What the weather like tomorrow ?")
    assert has_no_css?(".interpretation-resume")
  end


  test "Update an interpretation and cancel" do
    admin_go_to_intent_show(agents(:weather), intents(:weather_forecast))

    within(".card .tabs") do
      click_link "en"
    end

    assert has_link?("What the weather like tomorrow ?")

    within("#interpretations-list") do
      click_link "What the weather like tomorrow ?"
      assert has_text?("Cancel")
      click_link("Cancel")
    end
    assert has_link?("What the weather like tomorrow ?")
  end


  test "change locale via drag & drop" do
    admin_go_to_intent_show(agents(:weather), intents(:weather_forecast))

    expected = ["No language 0", "en 1", "fr 1", "es 0", "+"]
    assert_equal expected, all(".card > .tabs ul li").collect(&:text)

    within '.tabs' do
      click_link "en"
    end

    source = first("#interpretations-list .card-list__item__draggable")
    target = all(".tabs li.js-draggable-locale a")[1]
    source.drag_to(target)

    sleep(0.5)

    expected = ["No language 0", "en 0", "fr 2", "es 0", "+"]
    assert_equal expected, all(".card > .tabs ul li").collect(&:text)
  end


  test "Delete an interpretation" do
    admin_go_to_intent_show(agents(:weather), intents(:weather_forecast))

    within(".card .tabs") do
      click_link "en"
    end

    assert has_link?("What the weather like tomorrow ?")
    assert_equal "1", first("#current-locale-tab-badge").text

    within("#interpretations-list") do
      click_link "What the weather like tomorrow ?"
      assert has_text?("Cancel")
      all("a").last.click
    end
    assert has_no_link?("Cancel")
    assert_equal "0", first("#current-locale-tab-badge").text
  end


  test "Create an interpretation with regex" do
    admin_go_to_intent_show(agents(:terminator), intents(:terminator_find))

    assert has_link?("Where is Sarah Connor ?")

    first("trix-editor").click.set("Detect Sarah")
    select_text_in_trix("trix-editor", 7, 12)

    within "#popup-add-tag" do
      assert has_link?("Regex")
      click_link("Regex")
    end

    within(".aliases") do
      assert has_link?("Railroad Diagram")
      assert has_text?("Sarah")
      assert has_text?("Blank Regex")

      find("textarea[name*='reg_exp']").set("[[[")
      assert has_text?("Invalid Regex")

      find("textarea[name*='reg_exp']").set("^[a-zA-z-]")
      assert has_text?("Valid Regex")
    end

    click_button "Add"

    assert has_link?("Detect Sarah")

    click_link("Detect Sarah")
    assert has_text?("Regex")
    expected_regex = "^[a-zA-z-]"
    assert_equal expected_regex, find("textarea[name*='reg_exp']").value
    assert has_text?("Valid Regex")
  end

  test "List every languages available" do
    admin_go_to_intent_show(agents(:terminator), intents(:terminator_find))
    click_link "+"

    within ".modal__main__chooser" do
      assert has_text? "No language"
      assert has_text? "fr (French)"
      assert has_text? "es (Spanish)"
      assert has_text? "pt (Portuguese)"
      assert has_text? "zh (Chinese)"
      assert has_text? "ar (Arabic)"
      assert has_text? "tr (Turkish)"

      assert has_no_text? "en (English)"
    end
  end

  private

    def admin_go_to_intent_show(agent, intent)
      admin_login
      visit user_agent_intent_path(agent.owner, agent, intent)
      visibility = intent.is_public? ? "PUBLIC" : "PRIVATE"
      assert has_text?("Interpretations / #{intent.intentname} #{visibility}")
    end

    def fill_in_editor_field(text)
      within ".CodeMirror" do
        # Click makes CodeMirror element active:
        current_scope.click
        # Find the hidden textarea:
        field = current_scope.find("textarea", visible: false)
        # Mimic user typing the text:
        field.send_keys text
      end
    end
end
