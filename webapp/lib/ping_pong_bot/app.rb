require 'sinatra/base'
require "sinatra/json"
require 'sinatra/reloader'
require 'rest-client'
require 'json'

class PingPongBot < Sinatra::Application
  configure :development do
    register Sinatra::Reloader
  end

  post '/start' do
    parameters  = JSON.parse(request.body.read)

    BotHelper.send_text(parameters["session_id"], "Hello")
    json status: 'ok'
  end

  post '/sessions/:session_id/user_statements' do
    sleep(0.5)

    session_id = params["session_id"]
    parameters  = JSON.parse(request.body.read)
    user_statement_says = parameters["user_statement"]["says"]

    case user_statement_says

    when /ping/i
      BotHelper.send_text(session_id, "Pong")

    when /pong/i
      BotHelper.send_text(session_id, "Ping")

    when /images/i
      (0..2).each do |i|
        BotHelper.send_image(session_id, BotHelper.kittens[i], "Kittens ##{i}")
      end

    when /image/i
      BotHelper.send_image(
        session_id,
        BotHelper.kittens.sample,
        'Here we love kittens',
        'The kittens are too cute. Do you agree?'
      )
    else
      BotHelper.send_text(session_id, "Received that user says: \"#{user_statement_says}\"")
    end

    json status: 'ok'
  end
end




class BotHelper

  def self.send_text(session_id, message)
    post_to_viky_ai(
      session_id,
      build_text_params(message)
    )
  end

  def self.send_image(session_id, url, title = '', subtitle = '')
    post_to_viky_ai(
      session_id,
      build_image_params(url, title, subtitle)
    )
  end

  def self.kittens
    [
      'https://www.wanimo.com/veterinaire/images/articles/chat/chaton-sourire.jpg',
      'https://static.wamiz.fr/images/news/facebook/article/lucy-acc-fb-59831efb7b842.jpg',
      'http://img.bfmtv.com/c/1256/708/92a/5a15ebccb41456c68539f83aeba6d.jpeg',
      'https://cf.ltkcdn.net/cats/images/orig/211655-2121x1414-Kitten-peeking.jpg',
      'http://dirtypawzpetgrooming.com/wp-content/uploads/2012/01/kitten-little.jpg'
    ]
  end


  private

    def self.post_to_viky_ai(session_id, parameters)
      url = "http://localhost:3000/api/v1/chat_sessions/#{session_id}/statements"
      RestClient.post(url, parameters.to_json, content_type: :json, accept: :json)
    end

    def self.build_text_params(message)
      {
        statement: {
          nature: 'text',
          content: {
            text: message
          }
        }
      }
    end

    def self.build_image_params(url, title = '', subtitle = '')
      {
        statement: {
          nature:  'image',
          content: {
            url: url,
            title: title,
            subtitle: subtitle
          }
        }
      }
    end

end
