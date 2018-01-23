namespace :doc do

  desc "Build doc static site (for deployment)"
  task :build do
    Bundler.with_clean_env do
      `cd ../doc && bundle install && bundle exec middleman build`
    end
  end

  desc "Start doc server (for development)"
  task :server do
    Bundler.with_clean_env do
      `cd ../doc && bundle install && bundle exec middleman server`
    end
  end

end
