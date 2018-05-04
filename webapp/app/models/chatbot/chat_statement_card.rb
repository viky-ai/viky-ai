class Chatbot::ChatStatementCard
  include ActiveModel::Model
  include Speechable

  attr_accessor :components
end
