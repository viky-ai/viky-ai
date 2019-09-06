require_relative 'lib/task'

namespace :packages do
  desc "Reinit all NLP instances"
  task :reinit => [:environment] do |t, args|
    Nlp::Package.reinit
  end

  desc "Dump a specific package"
  task :dump, [:agent_id] => [:environment] do |t, args|
    unless args.agent_id.present?
      Task::Print.error "Need an agent id"
      exit 0
    end
    begin
      agent = Agent.find(args.agent_id)
    rescue ActiveRecord::RecordNotFound
      Task::Print.error "Agent with id '#{args.agent_id}' does not exist"
      exit 0
    end
    puts JSON.pretty_generate(Nlp::Package.new(agent).full_json_export)
  end
end
