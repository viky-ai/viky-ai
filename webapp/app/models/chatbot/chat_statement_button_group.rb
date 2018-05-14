class Chatbot::ChatStatementButtonGroup
  include ActiveModel::Model
  include Speechable

  attr_accessor :buttons, :disable_on_click

  validates :buttons, presence: true, length: {
    maximum: 6, too_long: I18n.t('errors.chat_statement.buttons.too_long')
  }
  validates :disable_on_click, inclusion: { in: [true, false] }, allow_nil: true
  validate :recursive_validation

  def nature
    "button_group"
  end

  def buttons_as_components
    buttons.collect do |button|
      Chatbot::ChatStatementButton.new(button)
    end
  end

  def disable_on_click?
    self.disable_on_click == true
  end

  def disable(statement, button_index)
    self.buttons = self.buttons.each_with_index.collect do |button, i|
      properties = {}
      properties[:disabled] = true
      properties[:selected] = button_index.to_i == i
      button.merge(properties)
    end
    statement.content = self
    statement.save
  end


  private

    def recursive_validation
      if buttons.respond_to? :each
        buttons_as_components.each_with_index do |button, i|
          if button.invalid?
            errors.add("Content button ##{i}", button.errors.full_messages.join(', '))
          end
        end
      end
    end

end
