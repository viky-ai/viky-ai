class ChatSessionChannel < ApplicationCable::Channel
  def subscribed
    stream_from 'chat_session_channel'
  end
end
