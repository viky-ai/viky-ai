require "sinatra"
require "sinatra/json"
require 'rest-client'
require 'json'

set :root, File.dirname(__FILE__)

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
        content: "Hello"
      }
    }
  )

  json status: 'ok'
end

post '/user_statements' do
  parameters  = JSON.parse(request.body.read)
  session_id     = parameters["session_id"]
  user_statement = parameters["user_statement"]

  post_to_viky_ai(
    session_id,
    {
      statement: {
        nature: 'text',
        content: "Received that user says: #{user_statement}"
      }
    }
  )

  json status: 'ok'
end
