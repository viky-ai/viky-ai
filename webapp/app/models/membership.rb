class Membership < ApplicationRecord
  belongs_to :agent
  belongs_to :user, touch: true

  validates :rights, inclusion: { in: ['all', 'edit', 'show'] }
  validates :user_id, uniqueness: { scope: [:agent_id] }
end
