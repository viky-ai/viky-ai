require 'sinatra/base'
require 'sinatra/reloader'
require 'sinatra/json'
require 'rest-client'
require 'json'

module BotRessources
  def self.kittens
    [
    'https://images.unsplash.com/photo-1445499348736-29b6cdfc03b9?w=800&q=80',
    'https://images.unsplash.com/photo-1503844281047-cf42eade5ca5?w=800&q=80',
    'https://images.unsplash.com/photo-1467839024528-ac3042ac0ae7?w=800&q=80',
    'https://images.unsplash.com/photo-1481134803835-48d6de104072?w=800&q=80',
    'https://images.unsplash.com/photo-1517172527855-d7a4feea491b?w=800&q=80'
    ]
  end
end


module BotApi

  def self.text(session_id, content)
    post(session_id, Params::build('text', content))
  end

  def self.image(session_id, content)
    post(session_id, Params::build('image', content))
  end

  def self.button(session_id, content)
    post(session_id, Params::build('button', content))
  end

  def self.post(session_id, parameters)
    base_url = ENV.fetch('VIKYAPP_BASEURL') { 'http://localhost:3000' }
    url = "#{base_url}/api/v1/chat_sessions/#{session_id}/statements"
    RestClient.post(url, parameters.to_json, content_type: :json, accept: :json)
  end

  class Params
    def self.build(nature, content)
      {
        statement: {
          nature: nature,
          content: content
        }
      }
    end
  end
end

class PingPongBot < Sinatra::Base
  set :root, File.dirname(__FILE__)
  set :port, 3001

  # set :environment, :production

  configure :development do
    register Sinatra::Reloader
  end


  post '/start' do
    content = ""
    content << "<p><strong>Welcome!</strong></p>"
    content << "<p>I'm the <em>Ping Pong Bot</em>, i allow you to test the viky.ai chatbot system.</p>"
    content << "<p>You can type these terms in order to view available components:</p>"
    content << "<ul>"
    content << "  <li><code>ping</code> or <code>pong</code></li>"
    content << "  <li><code>image</code> or <code>images</code></li>"
    content << "  <li><code>button</code></li>"
    content << "</ul>"
    content << "<p>Happy coding!</p>"


    session_id = JSON.parse(request.body.read)["session_id"]
    BotApi.text(session_id, {
      text: content,
      speech: {
        text: "Welcome to Ping Pong Bot",
        locale: "en-US"
      }
    })

    status 200
    json Hash.new
  end


  post '/sessions/:session_id/user_actions' do
    sleep(0.5)

    session_id = params["session_id"]
    parameters = JSON.parse(request.body.read)

    case parameters['user_action']['type']

    when "click"
      payload = parameters['user_action']['payload']
      nice_payload = JSON.pretty_generate(payload);
      BotApi.text(session_id, {
        text: "<p>You triggered with payload:</p><pre>#{nice_payload}</pre>"
      })

    when "says"
      user_statement_says = parameters['user_action']['text']

      case user_statement_says
      when /ping/i
        BotApi.text(session_id, {
          text: "Pong",
          speech: {
            text: "Pong succeed",
            locale: "en-US"
          }
        })

      when /pong/i
        BotApi.text(session_id, {
          text: "Ping",
          speech: {
            text: "Ping succeed",
            locale: "en-US"
          }
        })

      when /images/i
        (0..2).each do |i|
          BotApi.image(session_id, {
            url: BotRessources.kittens[i],
            title: "Kittens ##{i+1}"
          })
        end

      when /image/i
        BotApi.image(session_id, {
          url: BotRessources.kittens.sample,
          title: 'Here we love kittens',
          subtitle: 'The kittens are too cute. Do you agree?',
          speech: {
            text: "Voici une image de chatton",
            locale: "fr-FR"
          }
        })

      when /button/i
        random_id = Random.rand(100)
        BotApi.button(session_id, {
          text: "Button #{random_id}",
          payload: {
            date: DateTime.now,
            action: "action_#{random_id}"
          }
        })

      else
        BotApi.text(session_id, {
          text: "I did not understand: \"#{user_statement_says}\"",
          speech: {
            text: "Oops",
            locale: "en-US"
          }
        })

      end
    end

    status 200
    json Hash.new
  end


  get '/ping' do
    base_url = ENV.fetch('VIKYAPP_BASEURL') { 'http://localhost:3000' }
    url = "#{base_url}/api/v1/ping.json"

    ping_failed = true
    begin
      response = RestClient::Request.execute(
        method: :get,
        url: url,
        headers: { accept: :json },
        timeout: 5
      )
      if response.code == 200
        ping_failed = false
      else
        message = response.body
      end
    rescue Exception => e
      message = e.message
    rescue RestClient::ExceptionWithResponse => e
      message = e.response.body
    end

    if ping_failed
      status 500
      json message: message
    else
      status 200
      json Hash.new
    end
  end

  # start the server if ruby file executed directly
  run! if app_file == $0
end
