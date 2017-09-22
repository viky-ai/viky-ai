Rails.application.routes.draw do
  devise_for :users, :controllers => { :invitations => 'backend/invitations' }

  namespace :backend do
    resources :users, only: [:index, :destroy] do
      member do
        get :confirm_destroy
        get :reinvite
      end
    end
  end

  root to: "welcome#index"

  get 'style-guide', to: 'style_guide#index'
end
