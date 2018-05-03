class Chatbot::ChatStatementSpeech
  include ActiveModel::Model

  attr_accessor :text
  attr_accessor :locale

  validates :text, presence: true, length: { maximum: 500 }
  validates :locale, inclusion: { in: ChatSession.locales.keys.collect(&:to_s) }
end
