class Chatbot::ChatStatementCard
  include ActiveModel::Model
  include Speechable

  attr_accessor :components

  validates :components, presence: true, length: { maximum: 5, too_long: I18n.t('errors.chat_statement.card.too_long') }
  validate :recursive_validation

  private

    def recursive_validation
      statements = []
      components.each do |component|
        case component['nature']
        when 'text'
          statements << Chatbot::ChatStatementText.new(component['content'])
        when 'image'
          statements << Chatbot::ChatStatementImage.new(component['content'])
        when 'button'
          statements << Chatbot::ChatStatementButton.new(component['content'])
        else
          errors.add(:base, I18n.t('errors.chat_statement.invalid_nature'))
        end
      end
      statements.find_all(&:invalid?)
                .each {|statement| errors.add(:base, statement.errors.full_messages.join(', '))}
    end

end
