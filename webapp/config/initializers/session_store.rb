Rails.application.config.session_store :cookie_store, key: '_vikyai', domain: {
  production: URI.parse(ENV.fetch("VIKYAPP_BASEURL") { 'http://www.viky.ai' }).host,
  development: :all
}.fetch(Rails.env.to_sym, :all)




