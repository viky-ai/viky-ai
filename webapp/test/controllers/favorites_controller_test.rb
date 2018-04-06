require 'test_helper'

class FavoritesControllerTest < ActionDispatch::IntegrationTest

  #
  # Create
  #
  test 'Allow favorite agent' do
    sign_in users(:admin)
    post favorites_url,
         params: {
           agent_id: agents(:weather).id,
           format: :js
         }
    assert response.body.include?("Remove favorite")
  end


  test 'Forbid favorite agent' do
    sign_in users(:confirmed)
    post favorites_url,
         params: {
           agent_id: agents(:weather).id,
           format: :json
         }
    assert_response :forbidden
    assert response.body.include?("Unauthorized operation.")
  end


  #
  # Destroy access
  #
  test 'Allow remove favorite agent' do
    admin = users(:admin)
    weather = agents(:weather)
    fav = FavoriteAgent.create(user: admin, agent: weather)
    assert fav.save

    sign_in admin
    delete favorite_url(fav), params: { format: :js }
    assert response.body.include?("Add favorite")
  end


  test 'Forbid remove favorite agent' do
    admin = users(:admin)
    weather = agents(:weather)
    fav = FavoriteAgent.create(user: admin, agent: weather)
    assert fav.save

    sign_in users(:confirmed)
    delete favorite_url(fav)
    assert_equal 'Unauthorized operation.', flash[:alert]
    assert_redirected_to agents_url
  end
end
