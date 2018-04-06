require 'test_helper'

class ReadmesControllerTest < ActionDispatch::IntegrationTest
  #
  # New
  #
  test 'New access' do
    sign_in users(:edit_on_agent_weather)
    get new_user_agent_readme_path(agents(:weather).owner, agents(:weather))
    assert_response :success
  end

  test 'New forbidden' do
    sign_in users(:show_on_agent_weather)
    get new_user_agent_readme_path(agents(:weather).owner, agents(:weather))
    assert_redirected_to agents_url
    assert_equal 'Unauthorized operation.', flash[:alert]
  end

  #
  # Create
  #
  test 'Create access' do
    sign_in users(:edit_on_agent_weather)
    post user_agent_readme_url(agents(:weather).owner, agents(:weather)),
         params: {
           readme: { content: 'README Test' },
           format: :json
         }
    assert_redirected_to user_agent_path(agents(:weather).owner, agents(:weather))
    assert_nil flash[:alert]
    assert_equal "README has been successfully created.", flash[:notice]
  end

  test 'Create forbidden' do
    sign_in users(:show_on_agent_weather)
    post user_agent_readme_url(agents(:weather).owner, agents(:weather)),
         params: {
           readme: { content: 'README Test' },
           format: :json
         }
    assert_response :forbidden
    assert response.body.include?('Unauthorized operation.')
  end

  #
  # Edit
  #
  test 'Edit access' do
    readme = Readme.new(content: "README Test")
    readme.agent = agents(:weather)
    assert readme.save

    sign_in users(:edit_on_agent_weather)

    get edit_user_agent_readme_path(agents(:weather).owner, agents(:weather))
    assert_response :success
  end

  test 'Edit forbidden' do
    readme = Readme.new(content: "README Test")
    readme.agent = agents(:weather)
    assert readme.save

    sign_in users(:show_on_agent_weather)

    get edit_user_agent_readme_path(agents(:weather).owner, agents(:weather))
    assert_redirected_to agents_url
    assert_equal 'Unauthorized operation.', flash[:alert]
  end

  #
  # Update
  #
  test 'Update access' do
    readme = Readme.new(content: "README Test")
    readme.agent = agents(:weather)
    assert readme.save

    sign_in users(:edit_on_agent_weather)
    patch user_agent_readme_path(agents(:weather).owner, agents(:weather)),
         params: {
           readme: { content: 'README Test' },
           format: :json
         }
    assert_redirected_to user_agent_path(agents(:weather).owner, agents(:weather))
    assert_nil flash[:alert]
    assert_equal "README has been successfully updated.", flash[:notice]
  end

  test 'Update forbidden' do
    readme = Readme.new(content: "README Test")
    readme.agent = agents(:weather)
    assert readme.save

    sign_in users(:show_on_agent_weather)
    patch user_agent_readme_path(agents(:weather).owner, agents(:weather)),
         params: {
           readme: { content: 'README Test' },
           format: :json
         }
    assert_response :forbidden
    assert response.body.include?('Unauthorized operation.')
  end

  #
  # Confirm delete
  #
  test 'Confirm delete access' do
    readme = Readme.new(content: "README Test")
    readme.agent = agents(:weather)
    assert readme.save

    sign_in users(:edit_on_agent_weather)
    get confirm_destroy_user_agent_readme_path(agents(:weather).owner, agents(:weather))
    assert_response :success
  end

  test 'Confirm delete forbidden' do
    readme = Readme.new(content: "README Test")
    readme.agent = agents(:weather)
    assert readme.save

    sign_in users(:show_on_agent_weather)
    get confirm_destroy_user_agent_readme_path(agents(:weather).owner, agents(:weather))
    assert_redirected_to agents_url
    assert_equal 'Unauthorized operation.', flash[:alert]
  end

  #
  # Delete
  #
  test 'Delete access' do
    readme = Readme.new(content: "README Test")
    readme.agent = agents(:weather)
    assert readme.save

    sign_in users(:edit_on_agent_weather)
    delete user_agent_readme_path(agents(:weather).owner, agents(:weather))

    assert_redirected_to user_agent_path(agents(:weather).owner, agents(:weather))
    assert_nil flash[:alert]
  end

  test 'Delete forbidden' do
    readme = Readme.new(content: "README Test")
    readme.agent = agents(:weather)
    assert readme.save

    sign_in users(:show_on_agent_weather)
    delete user_agent_readme_path(agents(:weather).owner, agents(:weather))

    assert_redirected_to agents_url
    assert_equal 'Unauthorized operation.', flash[:alert]
  end

end
