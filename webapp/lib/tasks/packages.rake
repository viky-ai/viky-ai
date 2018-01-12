namespace :packages do
  desc "Push all agents to NLP"
  task :push_all => [:environment] do |t, args|

    # reduce log level of this task in production
    Rails.logger.level = :warn if Rails.env.production?

    Nlp::Package.push_all
  end
end
