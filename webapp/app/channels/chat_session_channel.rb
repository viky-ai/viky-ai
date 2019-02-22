class ChatSessionChannel < ApplicationCable::Channel
  def subscribed
    stop_all_streams
    stream_from "chat_session_channel_#{current_user.id}"
  end

  def unsubscribed
    stop_all_streams
  end
end
