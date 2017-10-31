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
        member do
          get :confirm_destroy
          get :confirm_transfer_ownership
          post :transfer_ownership
          get :search_users_for_transfer_ownership
          get :generate_token
        end
      end
    end
  end
  get 'agents', to: 'agents#index'

  require 'sidekiq/web'
  authenticate :user, lambda { |u| u.admin? } do
    mount Sidekiq::Web => '/backend/jobs'
  end

  get 'style-guide', to: 'style_guide#index'


  unauthenticated :user do
    root to: "marketing#index", as: :unauthenticated_root
  end

  authenticate :user do
    root to: 'agents#index', as: :authenticated_root
  end

  # API with versioning
  namespace :api do
    namespace :v1 do
      scope '/agents' do
        get '/:user_id/:id/interpret', to: 'nlp#interpret'
      end
    end
  end

  match "/404", to: "errors#not_found", via: :all
  match "/500", to: "errors#internal_error", via: :all
end
