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

  def self.text(text)
    Params.new(Params::build_text(text))
  end

  def self.image(url, title = '', description = '')
    Params.new(Params::build_image(url, title, description))
  end

  def self.video(youtube_id, title = '', description = '')
    Params.new(Params::build_video(youtube_id, title, description))
  end

  def self.map(params , title = '', description = '')
    Params.new(Params::build_map(params, title, description))
  end

  def self.button(name, action)
    Params.new(Params::build_button(name, action))
  end

  def self.button_group(buttons_list, disable_on_click = false)
    Params.new(Params::build_button_group(buttons_list, disable_on_click))
  end

  def self.card(components)
    Params.new(Params::build_card(components))
  end

  def self.list(items, orientation = :horizontal)
    Params.new(Params::build_list(items, orientation))
  end

  def self.update_locale(session_id, locale)
    parameters = {
      chat_session: {
        locale: locale
      }
    }
    base_url = ENV.fetch('VIKYAPP_BASEURL') { 'http://localhost:3000' }
    url = "#{base_url}/api/v1/chat_sessions/#{session_id}"
    RestClient.put(url, parameters.to_json, content_type: :json, accept: :json)
    self.text("Done").send(session_id)
  end

  class Params
    def initialize(statement)
      @statement = statement
    end

    def add_speech(text, local)
      @statement[:speech] = {
        text: text,
        local: local
      }
      self
    end

    def send(session_id)
      post(session_id, statement: @statement)
    end

    def self.build_text(text)
      {
        nature: 'text',
        content: { text: text }
      }
    end

    def self.build_button(name, payload)
      {
        nature: 'button',
        content: {
          text: name,
          payload: payload
        }
      }
    end

    def self.build_button_group(buttons_list, disable_on_click = false)
      group = {
        nature: 'button_group',
        content: {
          buttons: buttons_list.collect do |button|
            {
              text: button[0],
              payload: button[1]
            }
          end
        }
      }
      group[:content][:disable_on_click] = true if disable_on_click
      group
    end

    def self.build_image(url, title = '', description = '')
      img = {
        url: url
      }
      img[:title] = title unless title.empty?
      img[:description] = description unless description.empty?
      {
        nature: 'image',
        content: img
      }
    end

    def self.build_video(youtube_id, title = '', description = '')
      video = {
        params: youtube_id
      }
      video[:title] = title unless title.empty?
      video[:description] = description unless description.empty?
      {
        nature: 'video',
        content: video
      }
    end

    def self.build_map(params, title = '', description = '')
      map = {
        params: params
      }
      map[:title] = title unless title.empty?
      map[:description] = description unless description.empty?
      {
        nature: 'map',
        content: map
      }
    end

    def self.build_card(components)
      {
        nature: 'card',
        content: {
          components: components
        }
      }
    end

    def self.build_list(items, orientation = :horizontal)
      {
        nature: 'list',
        content: {
          orientation: orientation,
          items: items
        }
      }
    end


    private

      def post(session_id, parameters)
        base_url = ENV.fetch('VIKYAPP_BASEURL') { 'http://localhost:3000' }
        url = "#{base_url}/api/v1/chat_sessions/#{session_id}/statements"
        RestClient.post(url, parameters.to_json, content_type: :json, accept: :json)
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
</ul>
HTML

    text_2  = <<-HTML
<p>2. <strong>Map widget</strong></p>
<p>Enter the following commands in order to play with maps (via Google Maps Embed API):</p>
<ul>
  <li><code>map_place</code> show map via <strong>Place mode</strong>.</li>
  <li><code>map_directions</code> show map via <strong>Directions mode</strong>.</li>
  <li><code>map_search</code> show map via <strong>Search mode</strong>.</li>
  <li><code>map_view</code> show map via <strong>View mode</strong>.</li>
  <li><code>map_streetview</code> show map via <strong>Street View mode</strong>.</li>
</ul>
HTML

    text_3  = <<-HTML
<p>3. <strong>Button & Button group widget</strong></p>
<p>Enter the following commands in order to play with buttons:</p>
<ul>
  <li><code>button</code> show the button widget.</li>
  <li><code>button_group</code> show the button group widget.</li>
  <li><code>button_group_deactivable</code> show the button group widget with disable_on_click option enabled.</li>
</ul>
HTML

    text_4  = <<-HTML
<p>4. <strong>Card & List widget</strong></p>
<ul>
  <li><code>card</code> show standard card widget.</li>
  <li><code>card_video</code> show card widget with video.</li>
  <li><code>hlist</code> show list with horizontal orientation.</li>
  <li><code>hlist_card</code> show list of cards with horizontal orientation.</li>
  <li><code>vlist</code> show list with vertical orientation.</li>
</ul>
HTML

    text_5  = <<-HTML
<p>5. <strong>Change locale</strong></p>
<p>
  User <code>change_locale</code> in order to change speech to text locale.
  This functionality is only available under Chrome.
</p>
<p>Happy testing!</p>
HTML

    session_id = JSON.parse(request.body.read)["session_id"]
    BotApi
      .list(
        [
          BotApi::Params::build_text(text_1),
          BotApi::Params::build_text(text_2),
          BotApi::Params::build_text(text_3),
          BotApi::Params::build_text(text_4),
          BotApi::Params::build_text(text_5)
        ],
        :horizontal,
      )
      .add_speech('Welcome to Ping Pong Bot', 'en-US')
      .send(session_id)

    status 200
    json Hash.new
  end


  post '/sessions/:session_id/user_actions' do
    sleep(0.5)

    session_id = params["session_id"]
    parameters = JSON.parse(request.body.read)

    case parameters['user_action']['type']

    when 'click'
      payload = parameters['user_action']['payload']

      case payload['action']
      when 'display_kitten'
        BotApi.image(BotRessources.kittens.sample).send(session_id)
      when 'display_puppy'
        BotApi.image(BotRessources.puppies.sample).send(session_id)
      when 'display_duckling'
        BotApi.image(BotRessources.ducklings.sample).send(session_id)
      when 'update_locale'
        BotApi.update_locale(session_id, payload['locale'])
      else
        nice_payload = JSON.pretty_generate(payload)
        BotApi.text("<p>You triggered with payload:</p><pre>#{nice_payload}</pre>").send(session_id)
      end

    when "says"
      user_statement_says = parameters['user_action']['text']

      case user_statement_says

      when /ping/i
        BotApi
          .text('Pong')
          .add_speech('Pong succeed', 'en-US')
          .send(session_id)

      when /pong/i
        BotApi
          .text('Ping')
          .add_speech('Ping succeed', 'en-US')
          .send(session_id)

      when /image/i
        BotApi
          .image(
            BotRessources.kittens.sample,
            'Here we love kittens',
            'The kittens are too cute. Do you agree?'
          )
          .add_speech('Voici une image de chatton', 'fr-FR')
          .send(session_id)
        when /map(_|\s)?(place|directions|search|view|streetview)/i
          params, title, description = case $2
            when 'place'
              ["place?key=***REMOVED***&q=Valence",
              "Valence (Drôme)",
              "Valence est une commune du sud-est de la France. Avec 62 150 habitants, elle est la ville la plus peuplée de la Drôme. Ses habitants sont appelés les Valentinois."]
            when 'directions'
              ["directions?key=***REMOVED***&origin=Paris+France&destination=Valence+France",
              "Itinéraire de Paris à Valence", '']
            when 'search'
              ["search?key=***REMOVED***&q=Restaurant+Valence",
              "Restaurants (Valence)", '']
            when 'view'
              ["view?key=***REMOVED***&center=48.858281,2.294667&zoom=18&maptype=satellite",
              "Tour Eiffel",
              "Célèbre tour en fer de Gustave Eiffel (1889), terrasses panoramiques accessibles par escaliers et ascenseurs."]
            when 'streetview'
              ["streetview?key=***REMOVED***&location=44.929228,4.8887884&heading=-60&pitch=10",
              "Kiosque Peynet (Valence)", '']
            else
              ['', '', '']
          end
          BotApi.map(params, title, description).send(session_id)

      when /change_locale/i
          BotApi.card(
            [
              BotApi::Params::build_text('<p>Choose speech to text locale:</p>'),
              BotApi::Params::build_button_group(
                [
                  ['fr-FR', { action: 'update_locale', locale: 'fr-FR' }],
                  ['en-US', { action: 'update_locale', locale: 'en-US' }],
                  ['it-IT', { action: 'update_locale', locale: 'it-IT' }],
                  ['ar',    { action: 'update_locale', locale: 'ar' }],
                  ['ko-KR', { action: 'update_locale', locale: 'ko-KR' }]
                ],
                false
              )
            ]
          ).send(session_id)

      when /button(_|\s)?(group)?(_|\s)?(deactivable)?/i
        is_group = !$2.nil? && !$2.empty?
        is_deactivable = !$4.nil? && !$4.empty?
        if is_group
          BotApi.button_group([
            ['Show me kitten',   { action: 'display_kitten' }],
            ['Show me puppy',    { action: 'display_puppy'  }],
            ['Show me duckling', { action: 'display_duckling' }]
          ], is_deactivable).send(session_id)
        else
          random_id = Random.rand(100)
          BotApi.button("Button #{random_id}", { action: "action_#{random_id}"}).send(session_id)
        end

      when /hlist(_|\s)?(card)?/i
        is_card = !$2.nil? && !$2.empty?
        if is_card
          BotApi.list(
            [
              BotApi::Params::build_card([
                BotApi::Params::build_image(BotRessources.kittens[0], 'Lovely kitten - 780$', 'Soooooo cute!'),
                BotApi::Params::build_button('Add to basket', 'kitten_0_added_to_basket')
              ]),
              BotApi::Params::build_card([
                BotApi::Params::build_image(BotRessources.kittens[1], 'Lovely kitten - 600$', 'Soooooo cute!'),
                BotApi::Params::build_button('Add to basket', 'kitten_1_added_to_basket')
              ]),
              BotApi::Params::build_card([
                BotApi::Params::build_image(BotRessources.kittens[2], 'Lovely kitten - 1200$', 'Soooooo cute!'),
                BotApi::Params::build_button('Add to basket', 'kitten_2_added_to_basket')
              ]),
            ],
            :horizontal
          ).send(session_id)
        else
          BotApi
            .list(
              BotRessources.kittens.collect { |img| BotApi::Params::build_image(img) },
              :horizontal
            )
            .add_speech('Here is an horizontal list of kittens', 'en-US')
            .send(session_id)
        end

      when /vlist/i
        BotApi
          .list([
            BotApi::Params::build_image(BotRessources.puppies[0]),
            BotApi::Params::build_image(BotRessources.puppies[1]),
            BotApi::Params::build_text("<strong>What's your favorite?</strong>"),
            BotApi::Params::build_button_group([
              ['The first', 'choose_puppy_0'],
              ['The second', 'choose_puppy_1']
            ],
              true)
            ],
            :vertical)
          .add_speech('Here is an vertical list of mixed content', 'en-US')
          .send(session_id)

      when /card(_|\s)?(video)?/i
        is_video = !$2.nil? && !$2.empty?
        if is_video
          BotApi.card([
            BotApi::Params::build_video('bpOSxM0rNPM'),
            BotApi::Params::build_button('Buy the album', 'album_added_to_basket')
          ]).send(session_id)
        else
          BotApi.card([
            BotApi::Params::build_image(BotRessources.kittens.sample, 'Lovely kitten - 780$', 'Soooooo cute!'),
            BotApi::Params::build_button('Add to basket', 'kitten_added_to_basket')
          ]).send(session_id)
        end

      when /video/i
        description = 'Arctic Monkeys are an English rock band formed in 2002 in High Green'
        description << ', a suburb of Sheffield. Arctic Monkeys new album Tranquility Base '
        description << 'Hotel & Casino is out now on Domino Record Co.'
        BotApi
          .video(
            'bpOSxM0rNPM',
            'Arctic Monkeys - Do I Wanna Know?',
            description
          )
          .add_speech("Let's play musuc video!", 'en-GB')
          .send(session_id)
      else
        BotApi
          .text("I did not understand: \"#{user_statement_says}\"")
          .add_speech('Oops', 'en-US')
          .send(session_id)
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
    rescue RestClient::ExceptionWithResponse => e
      message  = "Body: #{e.response.body}"
      message << "Exception message: #{e.message}"
    rescue Exception => e
      message = "Exception message: #{e.message}"
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
