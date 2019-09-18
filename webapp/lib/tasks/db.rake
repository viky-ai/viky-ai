require_relative 'lib/task'

namespace :db do

  desc "Keeps only users related agents - Usage: rails db:truncate[toto@blague.com,titi@grosminet.com]"
  task :truncate => [:environment] do |t, args|
    unless args.extras.count > 0
      Task::Print.error "Need user emails"
      exit 0
    end

    emails = args.extras
    emails.each do |email|
      if User.find_by_email(email).nil?
        Task::Print.error "User with email: #{email} doesn't exist."
        exit 0
      end
    end

    agents_to_keep = []
    users_to_keep  = []

    Nlp::Package.sync_active = false

    emails.each do |email|
      user = User.find_by_email(email)
      users_to_keep << user.id
      user.agents.includes(:memberships).where('memberships.rights' => 'all').each do |agent|
        graph = AgentGraph.new(agent).to_graph
        users_to_keep  << graph.vertices.collect(&:owner_id)
        agents_to_keep << graph.vertices.collect(&:id)
      end
    end
    users_to_keep  = users_to_keep.flatten.uniq
    agents_to_keep = agents_to_keep.flatten.uniq


    Task::Print.step "Delete agents"
    Agent.find_each.each do |agent|
      unless agents_to_keep.include? agent.id
        Task::Print.substep "Delete agent: #{agent.slug}"
        agent.agent_regression_checks.delete_all
        agent.memberships.each { |m| m.destroy }
        agent.destroy
      end
    end

    Task::Print.step "Delete users"
    User.find_each.each do |user|
      unless users_to_keep.include? user.id
        Task::Print.substep "Delete user: #{user.email}"
        user.destroy
      end
    end

  end

end
