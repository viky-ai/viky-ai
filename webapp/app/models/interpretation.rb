class Interpretation < ApplicationRecord

  belongs_to :intent

  validates :expression, presence: true
end
