require 'application_system_test_case'

class InterpretationsTest < ApplicationSystemTestCase

  def select_text_in_trix(selector, position_start, position_end)
    script = "$('#{selector}').first().trigger('select-text', [#{position_start},#{position_end}])"
    page.execute_script(script)
  end


  test 'Create an interpretation' do
    go_to_agents_index
    assert page.has_text?('admin/weather')
    click_link 'My awesome weather bot admin/weather'
    assert page.has_text?('weather_greeting')

    click_link 'weather_greeting'
    assert page.has_text?('Add')

    assert_equal "1", first('#current-locale-tab-badge').text

    first('trix-editor').click.set('Good morning')
    click_button 'Add'
    assert page.has_text?('Good morning')

    assert_equal "2", first('#current-locale-tab-badge').text
  end


  test 'Create an interpretation with alias' do
    go_to_agents_index
    assert page.has_text?('admin/weather')
    click_link 'My awesome weather bot admin/weather'
    assert page.has_text?('weather_greeting')

    click_link 'weather_greeting'
    assert page.has_text?('Add')

    assert_equal 1, all('.interpretation-resume').count
    assert_equal "world", first('.interpretation-resume__alias-blue').text

    first('trix-editor').click.set('Salut Marcel')
    select_text_in_trix("trix-editor", 6, 12)
    find_link('admin/weather/weather_who').click

    within('.aliases') do
      assert page.has_text?('admin/weather/weather_who')
      assert page.has_text?('Marcel')
    end

    click_button 'Add'

    assert page.has_text?('Salut Marcel')
    assert_equal 2, all('.interpretation-resume').count
    assert_equal "Marcel", first('.interpretation-resume__alias-blue').text
  end


  test 'Create an interpretation with digit and any' do
    go_to_agents_index
    assert page.has_text?('admin/weather')
    click_link 'My awesome weather bot admin/weather'
    assert page.has_text?('weather_greeting')

    click_link 'weather_greeting'
    assert page.has_text?('Add')

    assert_equal 1, all('.interpretation-resume').count
    assert_equal "world", first('.interpretation-resume__alias-blue').text

    first('trix-editor').click.set("Le 01/01/2018 j'achète de l'Aspirine")
    select_text_in_trix("trix-editor", 3, 5)
    find_link('Digit').click

    select_text_in_trix("trix-editor", 6, 8)
    find_link('Digit').click

    select_text_in_trix("trix-editor", 9, 13)
    find_link('Digit').click

    select_text_in_trix("trix-editor", 23, 36)
    find_link('Any').click

    within('.aliases') do
      inputs = all("input[name*='aliasname']")
      inputs[0].set('day')
      inputs[1].set('month')
      inputs[2].set('year')
      inputs[3].set('product')
    end

    click_button 'Add'
    assert page.has_text?("Le 01/01/2018 j'achète de l'Aspirine")

    assert_equal 2, all('.interpretation-resume').count
    expected = ["01", "01", "2018", "de l'Aspirine"]
    assert_equal expected, all('.interpretation-resume__alias-black').collect(&:text)

    click_link "Le 01/01/2018 j'achète de l'Aspirine"
    assert page.has_text?('Cancel')
    within('.aliases') do
      expected = ["day", "month", "year", "product"]
      assert_equal expected, all("input[name*='aliasname']").collect(&:value)
    end
  end


  test 'Errors on interpretation creation' do
    go_to_agents_index
    assert page.has_text?('admin/weather')
    click_link 'My awesome weather bot admin/weather'
    assert page.has_text?('weather_greeting')

    click_link 'weather_greeting'
    assert page.has_text?('Add')

    assert_equal "1", first('#current-locale-tab-badge').text

    first('trix-editor').click.set('')
    click_button 'Add'
    assert page.has_text?('Expression can\'t be blank')

    assert_equal "1", first('#current-locale-tab-badge').text
  end


  test 'Errors on interpretation creation with alias' do
    go_to_agents_index
    assert page.has_text?('admin/weather')
    click_link 'My awesome weather bot admin/weather'
    assert page.has_text?('weather_greeting')

    click_link 'weather_greeting'
    assert page.has_text?('Add')

    assert_equal 1, all('.interpretation-resume').count
    assert_equal "world", first('.interpretation-resume__alias-blue').text

    first('trix-editor').click.set('Salut Marcel')
    select_text_in_trix("trix-editor", 6, 12)
    find_link('admin/weather/weather_who').click

    within('.aliases') do
      assert page.has_text?('admin/weather/weather_who')
      assert page.has_text?('Marcel')
      first('input').set("")
    end

    click_button 'Add'
    assert page.has_text?("Parameter name can't be blank")
  end


  test 'Update an interpretation (simple)' do
    go_to_agents_index
    assert page.has_text?('admin/weather')
    click_link 'My awesome weather bot admin/weather'
    assert page.has_text?('weather_greeting')

    click_link 'weather_greeting'
    click_link 'fr-FR'
    assert page.has_link?('Bonjour tout le monde')

    assert_equal '1', first('#current-locale-tab-badge').text

    within('#interpretations-list') do
      click_link 'Bonjour tout le monde'
      assert page.has_text?('Cancel')
      first('trix-editor').click.set('Salut à tous')
      check('interpretation[keep_order]')
      check('interpretation[glued]')
      fill_in_editor_field '10'
      click_button 'Update'
    end

    assert page.has_link?('Salut à tous')
    assert_equal '1', first('#current-locale-tab-badge').text
  end


  test 'Update an interpretation (add alias)' do
    go_to_agents_index
    assert page.has_text?('admin/weather')
    click_link 'My awesome weather bot admin/weather'
    assert page.has_text?('weather_greeting')

    click_link 'weather_greeting'
    click_link 'en-US'

    assert page.has_link?('Hello world')

    within('#interpretations-list') do
      click_link 'Hello world'
      assert page.has_text?('Cancel')

      select_text_in_trix("#interpretations-list trix-editor", 0, 5)
    end

    find_link('admin/weather/weather_who').click

    within('#interpretations-list') do
      within('.aliases') do
        assert page.has_text?('Hello')
      end

      click_button 'Update'
    end

    assert page.has_link?('Hello world')
    assert_equal 1, all('.interpretation-resume').count
    assert_equal ["Hello", "world"], all('.interpretation-resume__alias-blue').collect(&:text)
  end


  test 'Update an interpretation (remove alias)' do
    go_to_agents_index
    assert page.has_text?('admin/weather')
    click_link 'My awesome weather bot admin/weather'
    assert page.has_text?('weather_greeting')

    click_link 'weather_greeting'
    click_link 'en-US'

    assert page.has_link?('Hello world')

    within('#interpretations-list') do
      click_link 'Hello world'
      assert page.has_text?('Cancel')

      select_text_in_trix("#interpretations-list trix-editor", 6, 10)
    end
    find_link('Remove annotation(s)').click
    click_button 'Update'

    assert page.has_link?('Hello world')
    assert_equal 0, all('.interpretation-resume').count
  end


  test 'Update an interpretation and cancel' do
    go_to_agents_index
    assert page.has_text?('admin/weather')
    click_link 'My awesome weather bot admin/weather'
    assert page.has_text?('weather_greeting')

    click_link 'weather_greeting'
    click_link 'en-US'

    assert page.has_link?('Hello world')
    within('#interpretations-list') do
      click_link 'Hello world'
      assert page.has_text?('Cancel')
      click_link('Cancel')
    end
    assert page.has_link?('Hello world')
  end


  test 'Delete an interpretation' do
    go_to_agents_index
    assert page.has_text?('admin/weather')
    click_link 'My awesome weather bot admin/weather'
    assert page.has_text?('weather_greeting')

    click_link 'weather_greeting'
    assert page.has_link?('Hello world')

    assert_equal "1", first('#current-locale-tab-badge').text

    within('#interpretations-list') do
      click_link 'Hello world'
      assert page.has_text?('Cancel')
      all('a').last.click
    end
    assert page.has_no_link?('Cancel')
    assert_equal "0", first('#current-locale-tab-badge').text
  end


  private

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
