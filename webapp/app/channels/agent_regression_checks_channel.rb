class AgentRegressionChecksChannel < ApplicationCable::Channel
  def subscribed
    stream_from 'agent_regression_checks_channel'
  end
end
