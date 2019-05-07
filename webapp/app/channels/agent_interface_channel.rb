class AgentInterfaceChannel < ApplicationCable::Channel
  def subscribed
    stop_all_streams
    stream_from "agent_interface_channel_#{params[:agent_id]}"
  end

  def unsubscribed
    stop_all_streams
  end
end
