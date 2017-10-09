class Agent < ApplicationRecord
  has_many :memberships
  has_many :users, through: :memberships

  validates :name, presence: true
  validates :agentname, uniqueness: true, length: { in: 3..25 }, presence: true

  validates :users, presence: true
end
