class ChatStatementButtonGroup
  include ActiveModel::Model
  include Speechable

  attr_accessor :buttons
  validates :buttons, presence: true, length: { maximum: 6 }
  validate :recursive_validation


  private

    def recursive_validation
      unless buttons.nil?
        buttons.each_with_index do |button, i|
          button = ChatStatementButton.new(button)
          if button.invalid?
            button.errors.full_messages.each do |error|
              errors.add(:base, "Button ##{i}: #{error}")
            end
          end
        end
      end
    end

end
