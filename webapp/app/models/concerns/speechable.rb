module Speechable
  extend ActiveSupport::Concern

  included do
    attr_accessor :speech
    validate :speech_validation
  end

  def speech_validation
    if speech.is_a?(Hash)
      data = ChatStatementSpeech.new(speech)
      errors.add(:base, data.errors.full_messages.join(', ')) if data.invalid?
    elsif !speech.nil?
      errors.add(:speech, "must be a Hash")
    end
  end

end
