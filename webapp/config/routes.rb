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
      post :impersonate, on: :member
    end
  end


  resource :profile, only: [:show, :edit, :update, :destroy] do
    get :confirm_destroy
    post :stop_impersonating, on: :collection
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
          get :interpret, to: 'console#interpret'
        end
        get :search_users_to_share_agent, controller: 'memberships'

        resources :memberships, only: [:index, :new, :create, :update, :destroy] do
          get :confirm_destroy
        end

        resources :dependencies, only: [:new, :create, :destroy] do
          get :confirm_destroy
          collection do
            get :successors_graph
            get :predecessors_graph
          end
        end

        resources :intents, except: [:index] do
          get :select_new_locale
          post :add_locale
          delete :remove_locale
          get :confirm_destroy
          collection do
            post :update_positions
          end

          resources :interpretations, only: [:show, :create, :edit, :update, :destroy] do
            member do
              post :update_locale
            end
            collection do
              post :update_positions
            end
          end
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
  get 'style-guide/:page_id', to: "style_guide#page"

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
        get '/:ownername/:agentname/interpret', to: 'nlp#interpret'
      end
    end
  end

  match "/404", to: "errors#not_found", via: :all
  match "/500", to: "errors#internal_error", via: :all
end
