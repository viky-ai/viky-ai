Rails.application.routes.draw do
  devise_for :users

  root to: "application#index"

  get 'style-guide', to: 'style_guide#index'
end
