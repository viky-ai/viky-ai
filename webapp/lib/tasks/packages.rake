namespace :packages do
  desc "Push all agents to NLP"
  task :push_all => [:environment] do |t, args|
    Nlp::Package.push_all
  end
end
