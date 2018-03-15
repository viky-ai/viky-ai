class FavoriteAgent < ApplicationRecord
  belongs_to :agent
  belongs_to :user

  validates :agent_id, uniqueness: { scope: [:user_id] }
end
