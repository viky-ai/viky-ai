class Chatbot::Button
  include ActiveModel::Model
  include Speechable

  attr_accessor :text, :payload, :href, :disabled, :selected

  validates :text, presence: true, length: { maximum: 100 }
  validates :href, length: { maximum: 500 }
  validates :disabled, inclusion: { in: [true, false] }, allow_nil: true
  validates :selected, inclusion: { in: [true, false] }, allow_nil: true

  validate :validate_button_action

  def nature
    "button"
  end

  def is_disabled?
    self.disabled == true
  end

  def is_selected?
    self.selected == true
  end

  private

    def validate_button_action
      if @payload.present? && @href.present?
        errors.add(:href, I18n.t('errors.chat_statement.buttons.mutually_exclusive_payload_and_href'))
      end

      if @payload.blank? && @href.blank?
        errors.add(:base, I18n.t('errors.chat_statement.buttons.blank_payload_and_href'))
      end
    end
end
