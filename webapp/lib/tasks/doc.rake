require_relative 'lib/task'

doc_dir = ENV['VIKY_DOCUMENTATION_PATH']
return if doc_dir.blank?

namespace :doc do
  desc 'Build doc static site (for development)'
  task :build do
    if File.directory?(doc_dir)
      Bundler.with_clean_env do
        Task::Cmd.exec('bundle install', {}, doc_dir)
        Task::Cmd.exec('bundle exec jekyll build', {}, doc_dir)
      end
      if File.directory?(doc_dir + '/_site')
        Task::Cmd.exec "cp -R #{doc_dir}/_site/blog/ public/blog/"
        Task::Cmd.exec "cp -R #{doc_dir}/_site/doc/ public/doc/"
        Task::Cmd.exec "cp -R #{doc_dir}/_site/site_assets/ public/site_assets/"
        Task::Cmd.exec "cp #{doc_dir}/_site/index.html public/index.html"
      else
        Task::Print.error 'Jekyll did not build the documentation.'
      end
    else
      Task::Print.error "#{doc_dir} directory do not exists."
    end
  end
end
