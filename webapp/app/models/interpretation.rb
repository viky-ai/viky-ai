class Interpretation < ApplicationRecord
  Locales = %w[en_US fr_FR].sort.freeze

  belongs_to :intent

  validates :expression, presence: true
  validates :locale, inclusion: { in: self::Locales }, presence: true
end
