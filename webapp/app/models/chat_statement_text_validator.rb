class ChatStatementTextValidator

  attr_reader :errors

  def initialize(content)
    @content = content
    @errors = ''
  end

  def valid?
    validate
    @errors.empty?
  end

  def invalid?
    !valid?
  end

  def validate
    if @content.blank? || !@content.key?('text')
      @errors = I18n.t('errors.chat_statement.text.missing')
      return
    end
      @errors = I18n.t('errors.chat_statement.text.too_long') if @content['text'].size > 5000
      @errors = I18n.t('errors.chat_statement.text.blank') if @content['text'].blank?
  end
end
