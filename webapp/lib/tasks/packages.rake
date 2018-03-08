namespace :packages do
  desc "Reinit all NLP instances"
  task :reinit => [:environment] do |t, args|

    Nlp::Package.reinit
  end

  desc "Dump a specific package"
  task :dump, [:agent_id] => [:environment] do |t, args|
    unless args.agent_id.present?
      puts "#{time_log} " + Rainbow('Need an agent id').red
      exit 0
    end
    begin
      agent = Agent.find(args.agent_id)
    rescue ActiveRecord::RecordNotFound
      puts Rainbow("#{time_log} Agent with id '#{args.agent_id}' does not exist").red
      exit 0
    end
    nlp =
    puts JSON.pretty_generate(Nlp::Package.new(agent).full_json_export)
  end

  private
    def time_log
      "[#{DateTime.now.strftime("%FT%T")}]"
    end
end
