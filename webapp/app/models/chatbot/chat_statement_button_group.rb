class Chatbot::ChatStatementButtonGroup
  include ActiveModel::Model
  include Speechable

  attr_accessor :buttons, :disable_on_click

  validates :buttons, presence: true, length: { maximum: 6 }
  validates :disable_on_click, inclusion: { in: [true, false] }, allow_nil: true
  validate :recursive_validation

  def buttons_as_components
    buttons.collect do |button|
      Chatbot::ChatStatementButton.new(button)
    end
  end

  def disable_on_click?
    self.disable_on_click == true
  end


  private

    def recursive_validation
      unless buttons.nil?
        buttons.each_with_index do |button, i|
          button = Chatbot::ChatStatementButton.new(button)
          if button.invalid?
            button.errors.full_messages.each do |error|
              errors.add(:base, "Button ##{i}: #{error}")
            end
          end
        end
      end
    end

end
