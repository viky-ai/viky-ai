class Chatbot::Button
  include ActiveModel::Model
  include Speechable

  attr_accessor :text, :payload, :disabled, :selected

  validates :text, presence: true, length: { maximum: 100 }
  validates :payload, presence: true
  validates :disabled, inclusion: { in: [true, false] }, allow_nil: true
  validates :selected, inclusion: { in: [true, false] }, allow_nil: true

  def nature
    "button"
  end

  def is_disabled?
    self.disabled == true
  end

  def is_selected?
    self.selected == true
  end
end
