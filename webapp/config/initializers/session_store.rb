Rails.application.config.session_store :cookie_store, key: '_voqalai', domain: {
  production: '.viky.ai',
  development: :all
}.fetch(Rails.env.to_sym, :all)
