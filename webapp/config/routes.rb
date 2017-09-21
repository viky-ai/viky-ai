Rails.application.routes.draw do
  devise_for :users, :controllers => { :invitations => 'backend/invitations' }

  namespace :backend do
    resources :users, only: [:index]
  end

  root to: "welcome#index"

  get 'style-guide', to: 'style_guide#index'
end
