class Entity < ApplicationRecord
  belongs_to :entities_list, touch: true

  serialize :terms, JSON

  validates :solution, length: { maximum: 2000 }
  validates :terms, length: { maximum: 5000 }, presence: true
end
