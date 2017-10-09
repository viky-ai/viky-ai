class Membership < ApplicationRecord
  belongs_to :agent
  belongs_to :user
end

