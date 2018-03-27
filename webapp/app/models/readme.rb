class Readme < ApplicationRecord
  belongs_to :agent
  validates :content, presence: true
end
