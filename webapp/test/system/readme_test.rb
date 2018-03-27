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


  test 'Readme creation failure' do
    go_to_agent_show(users(:edit_on_agent_weather), agents(:weather))
    click_link 'Add a README'
    within('.modal') do
      assert page.has_text? 'Create README'
      fill_in 'readme_content', with: ''
      click_button 'Create'
      expected = ["Content can't be blank"]
      assert_equal expected, all('.help--error').collect(&:text)
    end
  end


  test 'Readme creation success' do
    go_to_agent_show(users(:edit_on_agent_weather), agents(:weather))
    click_link 'Add a README'
    within('.modal') do
      assert page.has_text? 'Create README'
      fill_in 'readme_content', with: 'README creation test.'
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
      fill_in 'readme_content', with: ''
      click_button 'Update'
      expected = ["Content can't be blank"]
      assert_equal expected, all('.help--error').collect(&:text)
    end
  end


  test 'Readme edit success' do
    go_to_agent_show(users(:admin), agents(:terminator))
    click_link 'Edit'
    within('.modal') do
      assert page.has_text? 'Edit README'
      fill_in 'readme_content', with: 'README creation update test.'
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


end
