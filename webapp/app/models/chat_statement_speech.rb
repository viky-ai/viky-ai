class ChatStatementSpeech
  include ActiveModel::Model

  attr_accessor :text
  attr_accessor :locale

  validates :text, presence: true, length: { maximum: 500 }
  validates :locale, inclusion: { in: ["en-US", "en-GB", "fr-FR", "es-ES", "it-IT"] }
end
