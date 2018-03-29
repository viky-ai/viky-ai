require "application_system_test_case"

class ReadmeTest < ApplicationSystemTestCase


  test 'Readme blankslate' do
    go_to_agent_show(users(:edit_on_agent_weather), agents(:weather))
    assert page.has_text?('There is no README defined for this agent')
  end


  test 'Readme show' do
    go_to_agent_show(users(:admin), agents(:terminator))
    assert page.has_text?('Readme for terminator agent')
  end


  test 'Readme creation default state' do
    go_to_agent_show(users(:edit_on_agent_weather), agents(:weather))
    click_link 'Add a README'
    within('.modal') do
      assert page.has_text? 'Create README'
      assert page.has_text? 'You can use this README to describe current agent. Write content in Mardown format.'
    end
  end


  test 'Readme creation live preview' do
    go_to_agent_show(users(:edit_on_agent_weather), agents(:weather))
    click_link 'Add a README'
    within('.modal') do
      assert page.has_text? 'Create README'
      fill_in_code_editor('# Hello')
      assert_equal "Hello", first('.markdown-editor__preview h1').text
    end
  end


  test 'Readme creation failure' do
    go_to_agent_show(users(:edit_on_agent_weather), agents(:weather))
    click_link 'Add a README'
    within('.modal') do
      assert page.has_text? 'Create README'
      fill_in_code_editor('')
      click_button 'Create'
    end
    assert page.has_text?("README can't be blank")
  end


  test 'Readme creation success' do
    go_to_agent_show(users(:edit_on_agent_weather), agents(:weather))
    click_link 'Add a README'
    within('.modal') do
      assert page.has_text? 'Create README'
      fill_in_code_editor('README creation test.')
      click_button 'Create'
    end
    assert page.has_text?('README has been successfully created.')
    assert page.has_text?('README creation test.')
  end


  test 'Readme edit failure' do
    go_to_agent_show(users(:admin), agents(:terminator))
    click_link 'Edit'
    within('.modal') do
      assert page.has_text? 'Edit README'
      fill_in_code_editor('')
      click_button 'Update'
    end
    assert page.has_text?("README can't be blank")
  end


  test 'Readme edit success' do
    go_to_agent_show(users(:admin), agents(:terminator))
    click_link 'Edit'
    within('.modal') do
      assert page.has_text? 'Edit README'
      fill_in_code_editor('README creation update test.')
      click_button 'Update'
    end
    assert page.has_text?('README has been successfully updated.')
    assert page.has_text?('README creation update test.')
  end


  test 'Readme deletion' do
    go_to_agent_show(users(:admin), agents(:terminator))

    click_link 'Delete'
    within('.modal') do
      assert page.has_text?('Are you sure?')
      click_button('Delete')
      assert page.has_text?('Please enter the text exactly as it is displayed to confirm.')

      fill_in 'validation', with: 'DELETE'
      click_button('Delete')
    end
    assert page.has_text?('README has successfully been deleted.')
  end

  private

    def fill_in_code_editor(text)
      page.execute_script(
        "var textArea= document.getElementById('readme_content');
         var editor = textArea.nextSibling.CodeMirror;
         editor.getDoc().setValue('#{text}');"
      )
    end

end
