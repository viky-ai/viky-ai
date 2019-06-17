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
        system 'cp -R ../../doc/_site/ public/'
      end
    else
      puts "#{doc_dir} directory do not exists."
    end
  end
end
