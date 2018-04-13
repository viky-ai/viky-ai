require 'test_helper'

class DuplicateAgentJobTest < ActiveJob::TestCase
  test 'Start the agent duplication job' do
    agent_count = Agent.count
    DuplicateAgentJob.perform_now(agents(:weather), users(:show_on_agent_weather))
    assert_equal agent_count + 1, Agent.count
    assert Agent.find_by(agentname: 'weather_copy')
  end
end
