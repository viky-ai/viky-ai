
def create_agent(name, user=:admin)
  agent = Agent.new(
    name: name,
    agentname: name.parameterize
  )
  agent.memberships << Membership.new(user_id: users(user).id, rights: 'all')
  assert agent.save
  agent
end
