class Entity < ApplicationRecord
  belongs_to :entities_list, touch: true

  serialize :terms, JSON

  validates :solution, length: { maximum: 2000 }
  validates :terms, length: { maximum: 5000 }, presence: true

  def terms=(value)
    result = value
    if value.instance_of?(String) && value.length <= 5000
      tokens = value.split("\n")
      result = tokens.reject(&:blank?)
                     .collect { |token| { 'term' => token } }
      result = nil if result.empty?
    end
    super(result)
  end
end
