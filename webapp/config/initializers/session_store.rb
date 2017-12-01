Rails.application.config.session_store :cookie_store, key: '_vikyai', domain: {
  production: URI.parse(ENV["VOQALAPP_BASEURL"]).host,
  development: :all
}.fetch(Rails.env.to_sym, :all)
