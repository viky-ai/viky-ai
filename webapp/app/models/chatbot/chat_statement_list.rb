class Chatbot::ChatStatementList
  include ActiveModel::Model
  include Speechable

  attr_accessor :items

  validates :items, presence: true, length: {
    maximum: 10, too_long: I18n.t('errors.chat_statement.list.too_long')
  }
  validate :recursive_validation

  def nature
    "list"
  end

  def items_as_components
    items.collect do |item|
      case item['nature']
      when 'text'
        Chatbot::ChatStatementText.new(item["content"])
      when 'image'
        Chatbot::ChatStatementImage.new(item["content"])
      when 'button'
        Chatbot::ChatStatementButton.new(item["content"])
      when 'button_group'
        Chatbot::ChatStatementButtonGroup.new(item["content"])
      end
    end
  end


  private

    def recursive_validation
      items.each do |item|
        unless ['text', 'image', 'button', 'button_group'].include? item["nature"]
          errors.add(:base, I18n.t('errors.chat_statement.invalid_nature', nature: item['nature']))
        end
      end
      items_as_components.each do |component|
        if !component.nil? && component.invalid?
          errors.add(:base, component.errors.full_messages.join(', '))
        end
      end
    end
end
