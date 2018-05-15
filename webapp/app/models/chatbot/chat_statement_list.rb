class Chatbot::ChatStatementList
  include ActiveModel::Model
  include Speechable

  attr_accessor :orientation, :items

  validates :orientation, inclusion: { in: %w(vertical horizontal) }, allow_nil: true
  validates :items, presence: true, length: {
    minimum: 2, too_short: I18n.t('errors.chat_statement.list.too_short'),
    maximum: 8, too_long: I18n.t('errors.chat_statement.list.too_long')
  }
  validate :recursive_validation

  def nature
    "list"
  end

  def is_horizontal?
    !is_vertival?
  end

  def is_vertival?
    orientation == "vertical"
  end

  def items_as_components
    items.collect do |item|
      case item['nature']
      when 'text'
        Chatbot::ChatStatementText.new(item["content"])
      when 'image'
        Chatbot::ChatStatementImage.new(item["content"])
      when 'video'
        Chatbot::ChatStatementVideo.new(item["content"])
      when 'button'
        Chatbot::ChatStatementButton.new(item["content"])
      when 'button_group'
        Chatbot::ChatStatementButtonGroup.new(item["content"])
      when 'card'
        Chatbot::ChatStatementCard.new(item["content"])
      end
    end
  end


  private

    def recursive_validation
      if items.respond_to? :each
        items.each do |item|
          unless ['text', 'image', 'video', 'button', 'button_group', 'card'].include? item["nature"]
            errors.add(:base, I18n.t('errors.chat_statement.invalid_nature', nature: item['nature']))
          end
        end
        items_as_components.each_with_index do |component, i|
          if !component.nil? && component.invalid?
            errors.add("Content item ##{i}", component.errors.full_messages.join(', '))
          end
        end
      end
    end
end
