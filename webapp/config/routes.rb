Rails.application.routes.draw do
  devise_for :users
  # For details on the DSL available within this file, see http://guides.rubyonrails.org/routing.html

  root to: "application#hello"

  get 'style-guide', to: 'style_guide#index'
end
