Rails.application.routes.draw do
  devise_for :users, controllers: {
    invitations: 'backend/invitations', registrations: 'registrations'
  }

  namespace :backend do
    resources :users, only: [:index, :destroy] do
      member do
        get :confirm_destroy
        get :reinvite
      end
    end
  end

  resource :profile, only: [:show, :edit, :update, :destroy] do
    get :confirm_destroy
  end

  scope '/agents' do
    resources :users, path: '', only: [] do
      resources :agents, path: '', except: [:index] do
        resources :memberships, only: [:index, :new, :create, :update, :destroy] do
          get :confirm_destroy
        end
        member do
          get :confirm_destroy
          get :confirm_transfer_ownership
          post :transfer_ownership
          get :search_users_for_transfer_ownership
        end
        get :search_users_to_share_agent, controller: 'memberships'
      end
    end
  end
  get 'agents', to: 'agents#index'

  require 'sidekiq/web'
  authenticate :user, lambda { |u| u.admin? } do
    mount Sidekiq::Web => '/backend/jobs'
  end

  get 'style-guide', to: 'style_guide#index'
  get 'style-guide/:page_id', to: "style_guide#page"

  unauthenticated :user do
    root to: "marketing#index", as: :unauthenticated_root
  end

  authenticate :user do
    root to: 'agents#index', as: :authenticated_root
  end

end
