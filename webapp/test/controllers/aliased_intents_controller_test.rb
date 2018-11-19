require 'test_helper'

class AliasedIntentsControllerTest < ActionDispatch::IntegrationTest

  test 'index from intents for edit access' do
    referer_index = user_agent_intents_url(
      users(:admin), agents(:terminator)
    )
    referer_show = user_agent_intent_path(
      users(:admin), agents(:terminator), intents(:simple_where)
    )
    sign_in users(:admin)

    assert users(:admin).can? :edit, agents(:terminator)

    # From index
    get user_agent_intent_aliased_intents_url(
      users(:admin), agents(:terminator), intents(:simple_where)
    ), headers: { "HTTP_REFERER": referer_index }

    assert_response :success

    # From show
    get user_agent_intent_aliased_intents_url(
      users(:admin), agents(:terminator), intents(:simple_where)
    ), headers: { "HTTP_REFERER": referer_show }

    assert_response :success
  end


  test 'index from intents for show access' do
    referer_index = user_agent_intents_url(
      users(:admin), agents(:terminator)
    )
    referer_show = user_agent_intent_path(
      users(:admin), agents(:terminator), intents(:simple_where)
    )
    sign_in users(:confirmed)

    assert users(:confirmed).can? :show, agents(:terminator)

    # From index
    get user_agent_intent_aliased_intents_url(
      users(:admin), agents(:terminator), intents(:simple_where)
    ), headers: { "HTTP_REFERER": referer_index }

    assert_response :success

    # From show
    get user_agent_intent_aliased_intents_url(
      users(:admin), agents(:terminator), intents(:simple_where)
    ), headers: { "HTTP_REFERER": referer_show }

    assert_response :success
  end


  test 'index from intents without access' do
    referer_index = user_agent_intents_url(
      users(:admin), agents(:weather)
    )
    referer_show = user_agent_intent_path(
      users(:admin), agents(:weather), intents(:weather_forecast)
    )
    sign_in users(:confirmed)

    assert_not users(:confirmed).can? :show, agents(:weather)

    # From index
    get user_agent_intent_aliased_intents_url(
      users(:admin), agents(:weather), intents(:weather_forecast)
    ), headers: { "HTTP_REFERER": referer_index }

    assert_equal "Unauthorized operation.", flash[:alert]

    # From show
    get user_agent_intent_aliased_intents_url(
      users(:admin), agents(:weather), intents(:weather_forecast)
    ), headers: { "HTTP_REFERER": referer_show }

    assert_equal "Unauthorized operation.", flash[:alert]
  end


  test 'index from entities_lists for edit access' do
    referer_index = user_agent_entities_lists_path(
      users(:admin), agents(:weather)
    )
    referer_show = user_agent_entities_list_path(
      users(:admin), agents(:weather), entities_lists(:weather_conditions)
    )
    sign_in users(:admin)

    assert users(:admin).can? :edit, agents(:weather)

    # From index
    get user_agent_entities_list_aliased_intents_path(
      users(:admin), agents(:weather), entities_lists(:weather_conditions)
    ), headers: { "HTTP_REFERER": referer_index }

    assert_response :success

    # From show
    get user_agent_entities_list_aliased_intents_path(
      users(:admin), agents(:weather), entities_lists(:weather_conditions)
    ), headers: { "HTTP_REFERER": referer_show }

    assert_response :success
  end


  test 'index from entities_lists for show access' do
    referer_index = user_agent_entities_lists_path(
      users(:admin), agents(:weather)
    )
    referer_show = user_agent_entities_list_path(
      users(:admin), agents(:weather), entities_lists(:weather_conditions)
    )
    sign_in users(:show_on_agent_weather)

    assert users(:show_on_agent_weather).can? :show, agents(:weather)

    # From index
    get user_agent_entities_list_aliased_intents_path(
      users(:admin), agents(:weather), entities_lists(:weather_conditions)
    ), headers: { "HTTP_REFERER": referer_index }

    assert_response :success

    # From show
    get user_agent_entities_list_aliased_intents_path(
      users(:admin), agents(:weather), entities_lists(:weather_conditions)
    ), headers: { "HTTP_REFERER": referer_show }

    assert_response :success
  end




  test 'index from entities_lists whithout access' do
    referer_index = user_agent_entities_lists_path(
      users(:admin), agents(:weather)
    )
    referer_show = user_agent_entities_list_path(
      users(:admin), agents(:weather), entities_lists(:weather_conditions)
    )
    sign_in users(:confirmed)

    assert_not users(:confirmed).can? :show, agents(:weather)

    # From index
    get user_agent_entities_list_aliased_intents_path(
      users(:admin), agents(:weather), entities_lists(:weather_conditions)
    ), headers: { "HTTP_REFERER": referer_index }

    assert_equal "Unauthorized operation.", flash[:alert]

    # From show
    get user_agent_entities_list_aliased_intents_path(
      users(:admin), agents(:weather), entities_lists(:weather_conditions)
    ), headers: { "HTTP_REFERER": referer_show }

    assert_equal "Unauthorized operation.", flash[:alert]
  end

end
