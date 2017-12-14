
def create_agent(name)
  agent = Agent.new(
    name: name,
    agentname: name.parameterize
  )
  agent.memberships << Membership.new(user_id: users(:admin).id, rights: 'all')
  assert agent.save
  agent
end
