class ChatStatementList
  include ActiveModel::Model
  include Speechable

  attr_accessor :list

  validates :list, presence: true, length: { maximum: 4, too_long: I18n.t('errors.chat_statement.list.too_long') }
  validate :recursive_validation


  private

    def recursive_validation
      list.each do |item|
        case item['nature']
        when "text"
          item.delete('nature')
          text = ChatStatementText.new(item)
          errors.add(:base, text.errors.full_messages.join(', ')) if text.invalid?
        else
          errors.add(:base, I18n.t('errors.chat_statement.invalid_nature'))
        end
      end
    end
end
