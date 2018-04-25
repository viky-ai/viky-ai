require 'sinatra/base'
require 'sinatra/reloader'
require 'sinatra/json'
require 'rest-client'
require 'json'


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
    'https://images.unsplash.com/photo-1445499348736-29b6cdfc03b9?w=800&q=80',
    'https://images.unsplash.com/photo-1503844281047-cf42eade5ca5?w=800&q=80',
    'https://images.unsplash.com/photo-1467839024528-ac3042ac0ae7?w=800&q=80',
    'https://images.unsplash.com/photo-1481134803835-48d6de104072?w=800&q=80',
    'https://images.unsplash.com/photo-1517172527855-d7a4feea491b?w=800&q=80'
    ]
  end

  def self.send_button(session_id, text, payload = {})
    post_to_viky_ai(
      session_id,
      build_button_params(text, payload)
    )
  end


  private

    def self.post_to_viky_ai(session_id, parameters)
      base_url = ENV.fetch('VIKYAPP_BASEURL') { 'http://localhost:3000' }
      url = "#{base_url}/api/v1/chat_sessions/#{session_id}/statements"
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

  def self.build_button_params(text, payload)
    {
      statement: {
        nature: 'button',
        content: {
          text: text,
          payload: payload
        }
      }
    }
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
    parameters = JSON.parse(request.body.read)

    BotHelper.send_text(parameters["session_id"], "Hello")

    status 200
    json Hash.new
  end


  post '/sessions/:session_id/user_actions' do
    sleep(0.5)

    session_id = params["session_id"]
    parameters = JSON.parse(request.body.read)

    user_statement_type = parameters['user_statement']['type']
    if user_statement_type == "click"
      user_statement_payload = parameters['user_statement']['payload']
      BotHelper.send_text(session_id, "You triggered with payload : #{user_statement_payload}")
      return
    end

    user_statement_says = parameters['user_statement']['text']

    case user_statement_says

    when /ping/i
      BotHelper.send_text(session_id, "Pong")

    when /pong/i
      BotHelper.send_text(session_id, "Ping")

    when /images/i
      (0..2).each do |i|
        BotHelper.send_image(session_id, BotHelper.kittens[i], "Kittens ##{i+1}")
      end

    when /image/i
      BotHelper.send_image(
        session_id,
        BotHelper.kittens.sample,
        'Here we love kittens',
        'The kittens are too cute. Do you agree?'
      )

    when /button/i
      random_id = Random.rand(100)
      BotHelper.send_button(
        session_id,
        "Button #{random_id}",
        date: DateTime.now,
        action: "action_#{random_id}"
      )

    else
      BotHelper.send_text(session_id, "Received that user says: \"#{user_statement_says}\"")
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
