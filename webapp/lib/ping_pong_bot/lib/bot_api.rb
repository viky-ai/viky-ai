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

  def self.button_mail(name, href)
    Params.new(Params::build_button_mail(name, href))
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

  def self.geolocation(name)
    Params.new(Params::build_geolocation(name))
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

    def add_speech(text, locale)
      @statement[:content][:speech] = {
        text: text,
        locale: locale
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

    def self.build_button_mail(name, href)
      {
        nature: 'button',
        content: {
          text: name,
          href: href
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

    def self.build_geolocation(name)
      {
        nature: 'geolocation',
        content: {
          text: name
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
