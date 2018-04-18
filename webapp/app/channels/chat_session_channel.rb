class ChatSessionChannel < ApplicationCable::Channel
  def subscribed
    stream_from "chat_session_channel_#{current_user.id}"
  end
end
