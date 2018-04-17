class Bot < ApplicationRecord
  belongs_to :agent

  validates :name, :endpoint, presence: true
end
