require 'test_helper'

class ChatStatementsControllerTest < ActionDispatch::IntegrationTest

  test 'acces when feature chatbot is disabled' do
    Feature.with_chatbot_disabled do
      assert_raise(ActionController::RoutingError) do
        post "/api/v1/chat_sessions/#{chat_sessions(:one).id}/statements.json", params: {
          statement: {
            nature: 'text',
            content: {
              text: 'Hello'
            }
          }
        }
      end
    end
  end


  test 'Bot send an answer to a user statement' do
    Feature.with_chatbot_enabled do
      post "/api/v1/chat_sessions/#{chat_sessions(:one).id}/statements.json", params: {
        statement: {
          nature: 'text',
          content: {
            text: 'Hello'
          }
        }
      }
      assert_equal '201', response.code
    end
  end


  test 'Bot send a map' do
    Feature.with_chatbot_enabled do
      post "/api/v1/chat_sessions/#{chat_sessions(:one).id}/statements.json", params: {
        statement: {
          nature: 'map',
          content: {
            params: { api_key: "missing", endpoint: "place", query: "q=Valence"},
            title: "Map title",
            description: "Map description"
          }
        }
      }
      assert_equal '201', response.code
    end
  end


  test 'Bot send a card with map' do
    Feature.with_chatbot_enabled do
      post "/api/v1/chat_sessions/#{chat_sessions(:one).id}/statements.json", params: {
        statement: {
          nature: 'card',
          content: {
            components: [
              {
                nature: 'map',
                content: {
                  params: { api_key: "missing", endpoint: "place", query: "q=Valence"},
                  title: "Map title",
                  description: "Map description"
                }
              }
            ]
          }
        }
      }
      assert_equal '201', response.code
    end
  end


  test 'Bot send a list with map' do
    Feature.with_chatbot_enabled do
      post "/api/v1/chat_sessions/#{chat_sessions(:one).id}/statements.json", params: {
        statement: {
          nature: 'list',
          content: {
            orientation: 'vertical',
            items: [
              {
                nature: 'text',
                content: {
                  text: "Hello"
                }
              },
              {
                nature: 'map',
                content: {
                  params: { api_key: "missing", endpoint: "place", query: "q=Valence"},
                  title: "Map title",
                  description: "Map description"
                }
              }
            ]
          }
        }
      }
      assert_equal '201', response.code
    end
  end


  test 'Bot send a list of cards with map' do
    Feature.with_chatbot_enabled do
      post "/api/v1/chat_sessions/#{chat_sessions(:one).id}/statements.json", params: {
        statement: {
          nature: 'list',
          content: {
            orientation: 'vertical',
            items: [
              {
                nature: 'text',
                content: {
                  text: "Hello"
                }
              },
              {
                nature: 'card',
                content: {
                  components: [
                    {
                      nature: 'map',
                      content: {
                        params: { api_key: "missing", endpoint: "place", query: "q=Valence"},
                        title: "Map title",
                        description: "Map description"
                      }
                    }
                  ]
                }
              }
            ]
          }
        }
      }
      assert_equal '201', response.code
    end
  end


  test 'Reject sending a statement to an old session' do
    Feature.with_chatbot_enabled do
      assert ChatSession.create(
        user: users(:admin),
        bot: bots(:weather_bot)
      )
      post "/api/v1/chat_sessions/#{chat_sessions(:one).id}/statements.json", params: {
        statement: {
          nature: 'text',
          content: {
            text: 'Hello'
          }
        }
      }
      assert_equal '403', response.code
      assert_equal '{"errors":"Expired chat session"}', response.body
    end
  end


  test 'Reject invalid statement' do
    Feature.with_chatbot_enabled do
      post "/api/v1/chat_sessions/#{chat_sessions(:one).id}/statements.json", params: {
        statement: {
          nature: 'text',
          content: ''
        }
      }
      assert_equal '422', response.code
      assert_equal '{"errors":["content can\'t be blank"]}', response.body
    end
  end


  test 'Reject invalid button group statement' do
    Feature.with_chatbot_enabled do
      post "/api/v1/chat_sessions/#{chat_sessions(:one).id}/statements.json", params: {
        statement: {
          nature: 'button_group',
          content: {
            buttons: [
              {
                text: "",
                payload: {}
              }
            ]
          }
        }
      }
      assert_equal '422', response.code
      rsp = JSON.parse(response.body)
      expected = ["Content button #0 content.text can't be blank, content payload and href can't both be blank"]
      assert_equal expected, rsp["errors"]
    end
  end


  test 'Allow anything has button payload' do
    Feature.with_chatbot_enabled do
      post "/api/v1/chat_sessions/#{chat_sessions(:one).id}/statements.json", params: {
        statement: {
          nature: 'button',
          content: {
            text: 'Button 1',
            payload: {
              string: 'foo bar',
              number: 5,
              boolean: true,
              nested_json: {
                random: 'content'
              }
            }
          }
        }
      }
      assert_equal '201', response.code
    end
  end
end
