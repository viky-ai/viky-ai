require 'application_system_test_case'

class InterpretationsTest < ApplicationSystemTestCase

  test 'navigation' do
    go_to_agents_index
    assert page.has_text?('admin/weather')
    click_link 'My awesome weather bot admin/weather'
    assert page.has_link?('Interpretations')
    click_link 'Interpretations'
    assert page.has_text?('weather_forecast')

    click_link 'weather_forecast'
    assert page.has_text?('Interpretations / weather_forecast PUBLIC')
  end


  test 'Show an interpretation with details' do
    login_as 'show_on_agent_weather@viky.ai', 'BimBamBoom'
    assert page.has_text?('admin/weather')
    click_link 'My awesome weather bot admin/weather'
    assert page.has_link?('Interpretations')
    click_link 'Interpretations'
    assert page.has_text?('weather_forecast')

    click_link 'weather_forecast'
    assert page.has_text?('Interpretations / weather_forecast PUBLIC')
    within('#interpretations-list') do
      click_link 'What the weather like tomorrow ?'
      assert page.has_text?('admin/weather/weather_question')
      assert page.has_link?('admin/weather/weather_question')
      assert page.has_no_button?('Update')
      assert page.has_no_link?('Delete')
      assert page.has_no_field?('trix-editor')
      assert page.has_no_field?("input[name*='aliasname']")
    end
  end


  test 'Create an interpretation' do
    admin_go_to_intent_show(agents(:weather), intents(:weather_forecast))

    assert_equal "1", first('#current-locale-tab-badge').text

    first('trix-editor').click.set('Good morning')
    click_button 'Add'
    assert page.has_text?('Good morning')

    assert_equal "2", first('#current-locale-tab-badge').text
  end


  test 'Create an interpretation with intent alias' do
    admin_go_to_intent_show(agents(:weather), intents(:weather_forecast))

    assert_equal 1, all('.interpretation-resume').count
    assert_equal "What the weather like", first('.interpretation-resume__alias-blue').text

    first('trix-editor').click.set('Salut Marcel')
    select_text_in_trix("trix-editor", 6, 12)
    find_link('admin/weather/interpretations/weather_question').click

    within('.aliases') do
      assert page.has_link?('admin/weather/interpretations/weather_question')
      assert page.has_text?('Marcel')
    end

    click_button 'Add'

    assert page.has_text?('Salut Marcel')
    assert_equal 2, all('.interpretation-resume').count
    assert_equal "Marcel", first('.interpretation-resume__alias-blue').text

    first('trix-editor').click.set('Il fait beau ?')
    select_text_in_trix("trix-editor", 9, 12)
    find_link('admin/weather/interpretations/weather_question').click

    within('#interpretations-form .aliases') do
      click_link 'admin/weather/interpretations/weather_question'
    end
    assert page.has_text?('Interpretations / weather_question PUBLIC')
  end


  test 'Create an interpretation with entities_list alias' do
    admin_go_to_intent_show(agents(:weather), intents(:weather_forecast))

    assert_equal 1, all('.interpretation-resume').count
    assert_equal "What the weather like", first('.interpretation-resume__alias-blue').text

    first('trix-editor').click.set('Y a-t-il du soleil ?')
    select_text_in_trix("trix-editor", 12, 18)
    find_link('admin/weather/entities_lists/weather_conditions').click

    within('.aliases') do
      assert page.has_link?('admin/weather/entities_lists/weather_conditions')
      assert page.has_text?('soleil')
    end

    click_button 'Add'

    assert page.has_text?('Y a-t-il du soleil ?')
    assert_equal 2, all('.interpretation-resume').count
    assert_equal "soleil", first('.interpretation-resume__alias-purple').text

    within('#interpretations-list') do
      click_link 'Y a-t-il du soleil ?'
    end
    within('.aliases') do
      find_link('admin/weather/entities_lists/weather_conditions').click
    end
    assert page.has_text?('Entities lists / weather_conditions PUBLIC')
  end


  test 'Create an interpretation with numbers' do
    admin_go_to_intent_show(agents(:weather), intents(:weather_forecast))

    assert_equal 1, all('.interpretation-resume').count
    assert_equal "What the weather like", first('.interpretation-resume__alias-blue').text

    first('trix-editor').click.set("Le 01/01/2018 j'achète de l'Aspirine")
    select_text_in_trix("trix-editor", 3, 5)
    find_link('Number').click

    select_text_in_trix("trix-editor", 6, 8)
    find_link('Number').click

    select_text_in_trix("trix-editor", 9, 13)
    find_link('Number').click

    within('.aliases') do
      inputs = all("input[name*='aliasname']")
      inputs[0].set('day')
      inputs[1].set('month')
      inputs[2].set('year')
    end

    click_button 'Add'
    assert page.has_text?("Le 01/01/2018 j'achète de l'Aspirine")

    assert_equal 2, all('.interpretation-resume').count
    expected = ["01", "01", "2018"]
    assert_equal expected, all('.interpretation-resume__alias-black').collect(&:text)

    click_link "Le 01/01/2018 j'achète de l'Aspirine"
    assert page.has_text?('Cancel')
    within('.aliases') do
      expected = ["day", "month", "year"]
      assert_equal expected, all("input[name*='aliasname']").collect(&:value)
    end
  end


  test 'Errors on interpretation creation' do
    admin_go_to_intent_show(agents(:weather), intents(:weather_forecast))

    assert_equal "1", first('#current-locale-tab-badge').text

    first('trix-editor').click.set('')
    click_button 'Add'
    assert page.has_text?('Expression can\'t be blank')

    assert_equal "1", first('#current-locale-tab-badge').text
  end


  test 'Errors on interpretation creation with alias' do
    admin_go_to_intent_show(agents(:weather), intents(:weather_forecast))

    assert_equal 1, all('.interpretation-resume').count
    assert_equal "What the weather like", first('.interpretation-resume__alias-blue').text

    first('trix-editor').click.set('Salut Marcel')
    select_text_in_trix("trix-editor", 6, 12)
    find_link('admin/weather/interpretations/weather_question').click

    within('.aliases') do
      assert page.has_text?('admin/weather/interpretations/weather_question')
      assert page.has_text?('Marcel')
      first('input').set("")
    end

    click_button 'Add'
    assert page.has_text?("Parameter name can't be blank")
  end


  test 'Update an interpretation (simple)' do
    admin_go_to_intent_show(agents(:weather), intents(:weather_forecast))

    within('.card') do
      click_link 'fr'
    end
    assert page.has_link?('Quel temps fera-t-il demain ?')

    assert_equal '1', first('#current-locale-tab-badge').text

    within('#interpretations-list') do
      click_link 'Quel temps fera-t-il demain ?'
      assert page.has_text?('Cancel')
      first('trix-editor').click.set('Salut à tous')
      check('interpretation[keep_order]')
      check('interpretation[glued]')
      uncheck('interpretation[auto_solution_enabled]')
      fill_in_editor_field '10'
      click_button 'Update'
    end

    assert page.has_link?('Salut à tous')
    assert_equal '1', first('#current-locale-tab-badge').text
  end


  test 'Update an interpretation (add alias)' do
    admin_go_to_intent_show(agents(:weather), intents(:weather_forecast))

    within('.card') do
      click_link 'en'
    end
    assert page.has_link?('What the weather like tomorrow ?')

    within('#interpretations-list') do
      click_link 'What the weather like tomorrow ?'
      assert page.has_text?('Cancel')
      select_text_in_trix("#interpretations-list trix-editor", 31, 32)
    end

    within('#popup-add-tag') do
      find_link('admin/weather/interpretations/weather_question').click
    end

    within('#interpretations-list') do
      within('.aliases') do
        assert page.has_text?('tomorrow')
        all("input[name*='is_list']").first.click
      end
      click_button 'Update'
    end

    assert page.has_link?('What the weather like tomorrow ?')
    assert_equal 1, all('.interpretation-resume').count
    assert_equal ['What the weather like', '?'], all('.interpretation-resume__alias-blue').collect(&:text)

    within('#interpretations-list') do
      click_link 'What the weather like tomorrow ?'
      assert page.has_text?('Cancel')
      assert all("input[name*='is_list']")[0].checked?
      assert !all("input[name*='is_list']")[1].checked?
    end
  end


  test 'Update an interpretation (remove alias)' do
    admin_go_to_intent_show(agents(:weather), intents(:weather_forecast))

    within('.card') do
      click_link 'en'
    end
    assert page.has_link?('What the weather like tomorrow ?')

    within('#interpretations-list') do
      click_link 'What the weather like tomorrow ?'
      assert page.has_text?('Cancel')
      select_text_in_trix("#interpretations-list trix-editor", 0, 21)
    end
    find_link('Remove annotation(s)').click
    click_button 'Update'

    assert page.has_link?('What the weather like tomorrow ?')
    assert_equal 1, all('.interpretation-resume').count
  end


  test 'Remove alias from summary board' do
    admin_go_to_intent_show(agents(:weather), intents(:weather_forecast))

    within('.card') do
      click_link 'en'
    end
    assert page.has_link?('What the weather like tomorrow ?')

    within('#interpretations-list') do
      click_link 'What the weather like tomorrow ?'
      within('.aliases') do
        assert page.has_text?('question')
        assert page.has_text?('dates')
        all('a[href="#"').last.click
        all('a[href="#"').last.click
        assert page.has_no_text?('question')
        assert page.has_no_text?('dates')
      end
      question = interpretation_aliases(:weather_forecast_tomorrow_question)
      assert_no_text_selected_in_trix question.interpretation.id, question.aliasname
      click_button 'Update'
    end

    assert page.has_link?('What the weather like tomorrow ?')
    assert_equal 0, all('.interpretation-resume').count
  end


  test 'Update an interpretation and cancel' do
    admin_go_to_intent_show(agents(:weather), intents(:weather_forecast))

    within('.card') do
      click_link 'en'
    end

    assert page.has_link?('What the weather like tomorrow ?')
    within('#interpretations-list') do
      click_link 'What the weather like tomorrow ?'
      assert page.has_text?('Cancel')
      click_link('Cancel')
    end
    assert page.has_link?('What the weather like tomorrow ?')
  end


  test 'change locale via drag & drop' do
    admin_go_to_intent_show(agents(:weather), intents(:weather_forecast))

    expected = ["en 1", "fr 1", "+"]
    assert_equal expected, all(".card > .tabs ul li").collect(&:text)

    # Does not works...

    # source = first('#interpretations-list .card-list__item__draggable)
    # target = first('.tabs li.js-draggable-locale')
    # source.drag_to(target)

    # expected = ["en 0", "fr 2", "+"]
    # assert_equal expected, all(".tabs ul li").collect(&:text)
  end


  test 'Delete an interpretation' do
    admin_go_to_intent_show(agents(:weather), intents(:weather_forecast))

    assert page.has_link?('What the weather like tomorrow ?')

    assert_equal "1", first('#current-locale-tab-badge').text

    within('#interpretations-list') do
      click_link 'What the weather like tomorrow ?'
      assert page.has_text?('Cancel')
      all('a').last.click
    end
    assert page.has_no_link?('Cancel')
    assert_equal "0", first('#current-locale-tab-badge').text
  end

  test 'Create an interpretation with regex' do
    admin_go_to_intent_show(agents(:terminator), intents(:terminator_find))

    assert page.has_link?('Where is Sarah Connor ?')

    first('trix-editor').click.set('Find Sarah')
    select_text_in_trix('trix-editor', 5, 10)
    assert page.has_link?('Regex')

    click_link('Regex')

    within('.aliases') do
      assert page.has_link?('Railroad Diagram')
      assert page.has_text?('Sarah')
      # fill_in("input[name*='reg_exp']", with: '^[a-zA-z-]')
      find("input[name*='reg_exp']").set('^[a-zA-z-]')
    end

    click_button 'Add'

    assert page.has_link?('Find Sarah')

    click_link('Find Sarah')
    assert page.has_text?('Regex')
    expected_regex = '^[a-zA-z-]'
    assert_equal expected_regex, find("input[name*='reg_exp']").value
  end


  private

    def admin_go_to_intent_show(agent, intent)
      admin_login
      visit user_agent_intent_path(users(:admin), agent, intent)
      assert page.has_text?("Interpretations / #{intent.intentname} PUBLIC")
    end

    def fill_in_editor_field(text)
      within '.CodeMirror' do
        # Click makes CodeMirror element active:
        current_scope.click
        # Find the hidden textarea:
        field = current_scope.find('textarea', visible: false)
        # Mimic user typing the text:
        field.send_keys text
      end
    end
end
