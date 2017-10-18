namespace :test do
  desc "Run all test"
  task :all => [:environment] do |t, args|
    Rake::Task["test:models"].invoke
    Rake::Task["test:helpers"].invoke
    Rake::Task["test:controllers"].invoke
    Rake::Task["test:mailers"].invoke
    Rake::Task["test:integration"].invoke
    Rake::Task["test:jobs"].invoke
    Rake::Task["test:system"].invoke
  end
end
