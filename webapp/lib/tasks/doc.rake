require_relative 'lib/task'

namespace :doc do
  desc "Build doc static site (for development)"
  task :build do
    doc_dir = '../../doc'
    if File.directory?(doc_dir)
      Dir.chdir(doc_dir) do
        Bundler.with_clean_env do
          system 'bundle install && bundle exec jekyll build'
        end
      end
      if File.directory?(doc_dir + "/_site")
        system 'cp -R ../../doc/_site/blog/ public/blog/'
        system 'cp -R ../../doc/_site/doc/ public/doc/'
        system 'cp -R ../../doc/_site/site_assets/ public/site_assets/'
        system 'cp  ../../doc/_site/index.html public/index.html'
      end
    else
      Task::Print.error "#{doc_dir} directory do not exists."
    end
  end
end
