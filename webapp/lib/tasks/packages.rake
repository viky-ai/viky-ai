namespace :packages do
  desc "Reinit all NLP instances"
  task :reinit => [:environment] do |t, args|

    Nlp::Package.reinit
  end
end
