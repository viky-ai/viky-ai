require 'test_helper'
require 'model_test_helper'

class FavoriteAgentTest < ActiveSupport::TestCase

  test 'Tag agent as favorite' do
    weather = agents(:weather)
    admin = users(:admin)
    assert FavoriteAgent.create(user: admin, agent: weather)
    assert_equal admin.favorites.count, 1
    assert_equal admin.favorites.first.id, weather.id
    assert_equal weather.fans.count, 1
    assert_equal weather.fans.first.id, admin.id
  end


  test 'Check uniqueness constraint of favorite agent' do
    agent = agents(:weather)
    user = users(:admin)
    assert FavoriteAgent.create(user: user, agent: agent)
    favorite = FavoriteAgent.new(user: user, agent: agent)
    exception = assert_raises do
      favorite.save!
    end
    assert exception.message.include? 'Agent has already been taken'
  end


  test 'Check integrity constraints of favorite agent' do
    unknown_user = User.new(id: '1afbac59-0df7-44ba-bcfc-40540475ff97', name: 'Unknown user')
    exception_user = assert_raises do
      FavoriteAgent.new(user: unknown_user, agent: agents(:weather)).save
    end
    assert exception_user.message.include? 'PG::ForeignKeyViolation'
    unknown_agent = Agent.new(
      id: '64b95d18-83b2-4c36-8629-7fbfd84add53',
      name: 'Unknown agent',
      agentname: 'unknown_agent'
    )
    exception_agent = assert_raises do
      FavoriteAgent.new(user: users(:confirmed), agent: unknown_agent).save
    end
    assert exception_agent.message.include? 'PG::ForeignKeyViolation'
  end


  test 'Delete favorite with user and agent' do
    public_agent = create_agent('Public agent 1')
    mrwho = User.create(email: 'not-admin@viky.ai', password: 'Hello baby', username: 'mrwho')
    favorite = FavoriteAgent.new(user: mrwho, agent: public_agent)
    assert favorite.save
    assert mrwho.destroy
    exception_user = assert_raises do
      favorite.reload
    end
    assert exception_user.message.include? "Couldn't find FavoriteAgent with"

    public_agent = create_agent('Public agent 2')
    mrwho = User.create(email: 'not-admin@viky.ai', password: 'Hello baby', username: 'mrwho')
    favorite = FavoriteAgent.new(user: mrwho, agent: public_agent)
    assert favorite.save
    assert public_agent.destroy
    exception_agent = assert_raises do
      favorite.reload
    end
    assert exception_agent.message.include? "Couldn't find FavoriteAgent with"
  end
end
