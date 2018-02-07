class Entity < ApplicationRecord
  belongs_to :entities_list, touch: true

  serialize :terms, JSON

  validates :solution, length: { maximum: 2000 }
  validates :terms, length: { maximum: 5000 }, presence: true
  validate :validate_locales_exists
  validate :validate_terms_present

  def terms=(value)
    result = value
    if value.instance_of?(String) && value.length <= 5000
      tokens = tokenize(value)
      normalized_tokens = set_default_locale(tokens)
      result = build_json(normalized_tokens)
    end
    super(result)
  end

  private
    def tokenize(value)
      value
        .split("\n")
        .reject(&:blank?)
        .collect do |token|
          parts = token.split(':')
          if parts.size > 2
            [parts[0..-2].join(':'), parts[-1]]
          else
            [parts[0], parts[1]]
          end
        end
    end

    def set_default_locale(tokens)
      tokens.collect do |array|
        locale = array[1].present? ? array[1] : Locales::ANY
        [array[0], locale ]
      end
    end

    def build_json(normalized_tokens)
      result = normalized_tokens.collect { |array| { 'term' => array[0], 'locale' => array[1] } }
      result.empty? ? nil : result
    end

    def validate_locales_exists
      return unless terms.instance_of?(Array)
      terms.each do |json|
        unless Locales::ALL.include?(json['locale'])
          errors.add(:terms, I18n.t('errors.entity.unknown_locale', current_locale: json['locale']))
        end
      end
    end

    def validate_terms_present
      return unless terms.instance_of?(Array)
      terms.each do |json|
        if json['term'].empty?
          errors.add(:terms, I18n.t('errors.entity.term_abscent'))
        end
      end
    end
end
