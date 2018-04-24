require "sinatra"
require "sinatra/json"
require 'rest-client'
require 'json'

set :root, File.dirname(__FILE__)
set :port, 3001

def post_to_viky_ai(session_id, parameters)
  url = "http://localhost:3000/api/v1/chat_sessions/#{session_id}/statements"
  RestClient.post(url, parameters.to_json, content_type: :json, accept: :json)
end

post '/start' do
  parameters  = JSON.parse(request.body.read)
  session_id = parameters["session_id"]

  post_to_viky_ai(
    session_id,
    {
      statement: {
        nature: 'text',
        content: {
          text: "Hello"
        }
      }
    }
  )

  json status: 'ok'
end

post '/sessions/:session_id/user_statements' do
  session_id = params["session_id"]

  parameters  = JSON.parse(request.body.read)
  user_statement_says = parameters["user_statement"]["says"]

  sleep(0.5)

  case user_statement_says
  when '#image_de_chaton'
    bot_statement = {
      statement: {
        nature:  'image',
        content: {
          url: ['https://www.wanimo.com/veterinaire/images/articles/chat/chaton-sourire.jpg',
                'https://static.wamiz.fr/images/news/facebook/article/lucy-acc-fb-59831efb7b842.jpg',
                'http://img.bfmtv.com/c/1256/708/92a/5a15ebccb41456c68539f83aeba6d.jpeg',
                'https://cf.ltkcdn.net/cats/images/orig/211655-2121x1414-Kitten-peeking.jpg',
                'http://dirtypawzpetgrooming.com/wp-content/uploads/2012/01/kitten-little.jpg'].sample,
          title: 'Kitten',
          subtitle: 'So cute !'
        }
      }
    }
  when '#image_python'
    bot_statement = {
      statement: {
        nature:  'image',
        content: {
          url: 'http://unknown',
          title: 'Kitten',
          subtitle: 'So cute !'
        }
      }
    }
  else
    bot_statement = {
      statement: {
        nature:  'text',
        content: {
          text: "Received that user says: #{user_statement_says}"
        }
      }
    }
  end

  post_to_viky_ai(
    session_id,
    bot_statement
  )

  json status: 'ok'
end
