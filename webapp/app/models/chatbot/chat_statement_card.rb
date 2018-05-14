class Chatbot::ChatStatementCard
  include ActiveModel::Model
  include Speechable

  attr_accessor :components, :payload

  validates :components, presence: true, length: { maximum: 5, too_long: I18n.t('errors.chat_statement.card.too_long') }
  validate :recursive_validation

  def nature
    "card"
  end

  def components_as_statements
    components.collect do |component|
      component_as_statement(component)
    end
  end

  private

  def recursive_validation
    components
      .each do |component|
        case component['nature']
        when 'text', 'image'
          next
        when 'button', 'button_group'
          errors.add(:base, I18n.t('errors.chat_statement.card.payload_conflict')) if payload.present?
        else
          errors.add(:base, I18n.t('errors.chat_statement.invalid_nature', nature: component['nature']))
        end
      end
      .map { |component| component_as_statement(component) }
      .reject(&:nil?)
      .find_all(&:invalid?)
      .each { |statement| errors.add(:base, statement.errors.full_messages.join(', ')) }
  end

  def component_as_statement(component)
    case component['nature']
    when 'text'
      Chatbot::ChatStatementText.new(component['content'])
    when 'image'
      Chatbot::ChatStatementImage.new(component['content'])
    when 'button'
      Chatbot::ChatStatementButton.new(component['content'])
    when 'button_group'
      Chatbot::ChatStatementButtonGroup.new(component['content'])
    end
  end

end
