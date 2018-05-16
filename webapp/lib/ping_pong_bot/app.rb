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

  def self.puppies
    [
      'https://images.unsplash.com/photo-1447684808650-354ae64db5b8?w=800&q=80',
      'https://images.unsplash.com/photo-1508946621775-9d59b75e074e?w=800&q=80',
      'https://images.unsplash.com/photo-1518914781460-a3ada465edec?w=800&q=80',
      'https://images.unsplash.com/photo-1456081445129-830eb8d4bfc6?w=800&q=80',
      'https://images.unsplash.com/photo-1521128591876-b4ace034003a?w=800&q=80'
    ]
  end

  def self.ducklings
    [
      'https://images.unsplash.com/photo-1442689859438-97407280183f?w=800&q=80',
      'https://images.unsplash.com/photo-1513039740139-de0804d85a20?w=800&q=80',
      'https://images.unsplash.com/photo-1516467790960-dfa408ed87be?w=800&q=80'
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

  def self.video(session_id, content)
    post(session_id, Params::build('video', content))
  end

  def self.map(session_id, content)
    post(session_id, Params::build('map', content))
  end

  def self.button(session_id, content)
    post(session_id, Params::build('button', content))
  end

  def self.button_group(session_id, content)
    post(session_id, Params::build('button_group', content))
  end

  def self.card(session_id, content)
    post(session_id, Params::build('card', content))
  end

  def self.list(session_id, content)
    post(session_id, Params::build('list', content))
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
    text_1  = <<-HTML
<p>1. <strong>Welcome!</strong></p>
<p>I'm the <em>Ping Pong Bot</em>, i allow you to test the viky.ai chatbot system.</p>
<p>Start by entering the following command:</p>
<ul>
  <li><code>ping</code> or <code>pong</code> show simple bot response.</li>
  <li><code>image</code> show the image widget.</li>
  <li><code>video</code> show the video widget.</li>
  <li><code>map</code> show the map widget.</li>
</ul>

HTML
    text_2  = <<-HTML
<p>2. <strong>Button & Button group widget</strong></p>
<p>Enter the following commands in order to play with buttons:</p>
<ul>
  <li><code>button</code> show the button widget.</li>
  <li><code>button_group</code> show the button group widget.</li>
  <li><code>deactivatable_button_group</code> show the button group widget with disable_on_click option enabled.</li>
</ul>
HTML

    text_3  = <<-HTML
<p>4. <strong>Card & List widget</strong></p>
<ul>
  <li><code>card</code> show standard card widget.</li>
  <li><code>card_video</code> show card widget with video.</li>
  <li><code>hlist</code> show list with horizontal orientation.</li>
  <li><code>hlist_card</code> show list of cards with horizontal orientation.</li>
  <li><code>vlist</code> show list with vertical orientation.</li>
</ul>
<p>Happy testing!</p>
HTML

    session_id = JSON.parse(request.body.read)["session_id"]
    BotApi.list(session_id, {
      orientation: :horizontal,
      items: [
        { nature: 'text', content: { text: text_1 } },
        { nature: 'text', content: { text: text_2 } },
        { nature: 'text', content: { text: text_3 } }
      ],
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

      case payload['action']
      when 'display_kitten'
        BotApi.image(session_id, {
          url: BotRessources.kittens.sample
        })
      when 'display_puppy'
        BotApi.image(session_id, {
          url: BotRessources.puppies.sample
        })
      when 'display_duckling'
        BotApi.image(session_id, {
          url: BotRessources.ducklings.sample
        })
      else
        nice_payload = JSON.pretty_generate(payload);
        BotApi.text(session_id, {
          text: "<p>You triggered with payload:</p><pre>#{nice_payload}</pre>"
        })
      end

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

      when /map/i
        BotApi.map(session_id, {
          params: "place?key=***REMOVED***&q=Valence"
        })

      when /deactivatable_button_group/i
        BotApi.button_group(session_id, {
          disable_on_click: true,
          buttons: [
            {
              text: "Show me kitten",
              payload: { action: "display_kitten" }
            },
            {
              text: "Show me puppy",
              payload: { action: "display_puppy" }
            },
            {
              text: "Show me duckling",
              payload: { action: "display_duckling" }
            }
          ]
        })

      when /button_group/i
        BotApi.button_group(session_id, {
          buttons: [
            {
              text: "Show me kitten",
              payload: { action: "display_kitten" }
            },
            {
              text: "Show me puppy",
              payload: { action: "display_puppy" }
            },
            {
              text: "Show me duckling",
              payload: { action: "display_duckling" }
            }
          ]
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

      when /card_video/i
        BotApi.card(session_id, {
          components: [
            {
              nature: 'video',
              content: {
                params: "bpOSxM0rNPM"
              }
            },
            {
              nature: 'button',
              content: {
                text: "Buy the album",
                payload: { action: "album_added_to_basket" }
              }
            }
          ],
        })


      when /hlist_card/i
        BotApi.list(session_id, {
          orientation: :horizontal,
          items: [
            {
              nature: 'card',
              content: {
                components: [
                  {
                    nature: 'image',
                    content: {
                      url: BotRessources.kittens[0],
                      title: "Lovely kitten - 780$",
                      subtitle: "Soooooo cute!"
                    }
                  },
                  {
                    nature: 'button',
                    content: {
                      text: "Add to basket",
                      payload: { action: "kitten_0_added_to_basket" }
                    }
                  }
                ]
              }
            },
            {
              nature: 'card',
              content: {
                components: [
                  {
                    nature: 'image',
                    content: {
                      url: BotRessources.kittens[1],
                      title: "Lovely kitten - 600$",
                      subtitle: "Soooooo cute!"
                    }
                  },
                  {
                    nature: 'button',
                    content: {
                      text: "Add to basket",
                      payload: { action: "kitten_1_added_to_basket" }
                    }
                  }
                ]
              }
            },
            {
              nature: 'card',
              content: {
                components: [
                  {
                    nature: 'image',
                    content: {
                      url: BotRessources.kittens[2],
                      title: "Lovely kitten - 1200$",
                      subtitle: "Soooooo cute!"
                    }
                  },
                  {
                    nature: 'button',
                    content: {
                      text: "Add to basket",
                      payload: { action: "kitten_2_added_to_basket" }
                    }
                  }
                ]
              }
            }
          ]
        })

      when /hlist/i
        BotApi.list(session_id, {
          orientation: :horizontal,
          items: BotRessources.kittens.collect { |img|
            {
              nature: 'image',
              content: { url: img }
            }
          },
          speech: {
            text: "Here is an horizontal list of kittens",
            locale: "en-US"
          }
        })


      when /vlist/i
        BotApi.list(session_id, {
          orientation: :vertical,
          items: [
            {
              nature: 'image',
              content: {
                url: BotRessources.puppies[0]
              }
            },
            {
              nature: 'image',
              content: {
                url: BotRessources.puppies[1]
              }
            },
            {
              nature: 'text',
              content: {
                text: "<strong>What's your favorite?</strong>"
              }
            },
            {
              nature: 'button_group',
              content: {
                disable_on_click: true,
                buttons: [
                  {
                    text: "The first",
                    payload: { action: "choose_puppy_0" }
                  },
                  {
                    text: "The second",
                    payload: { action: "choose_puppy_1" }
                  }
                ]
              }
            }
          ],
          speech: {
            text: "Here is an vertical list of mixed content",
            locale: "en-US"
          }
        })


      when /card/i
        BotApi.card(session_id, {
          components: [
            {
              nature: 'image',
              content: {
                url: BotRessources.kittens.sample,
                title: "Lovely kitten - 780$",
                subtitle: "Soooooo cute!"
              }
            },
            {
              nature: 'button',
              content: {
                text: "Add to basket",
                payload: { action: "kitten_added_to_basket" }
              }
            }
          ]
        })



      when /video/i
        subtitle  = 'Arctic Monkeys are an English rock band formed in 2002 in High Green'
        subtitle << ', a suburb of Sheffield. Arctic Monkeys new album Tranquility Base '
        subtitle << 'Hotel & Casino is out now on Domino Record Co.'
        BotApi.video(session_id, {
          params: "bpOSxM0rNPM",
          title: 'Arctic Monkeys - Do I Wanna Know?',
          subtitle: subtitle,
          speech: {
            text: "Let's play musuc video!",
            locale: "en-GB"
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
