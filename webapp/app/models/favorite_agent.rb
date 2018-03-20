class FavoriteAgent < ApplicationRecord
  belongs_to :agent
  belongs_to :user, touch: true

  validates :agent_id, uniqueness: { scope: [:user_id] }
end
