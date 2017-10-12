class Agent < ApplicationRecord
  extend FriendlyId
  friendly_id :agentname, use: :history, slug_column: 'agentname'

  has_many :memberships
  has_many :users, through: :memberships

  validates :name, presence: true
  validates :agentname, uniqueness: { scope: [:owner_id] }, length: { in: 3..25 }, presence: true
  validates :users, presence: true
  validates :owner_id, presence: true
  validate :owner_presence_in_users

  before_validation :add_owner_id, on: :create
  before_validation :clean_agentname

  def self.search(q = {})
    conditions = where("1 = 1")
    conditions = joins(:memberships).where("user_id = ?", q[:user_id])
    conditions = conditions.where("name LIKE ? OR agentname LIKE ?", "%#{q[:query]}%", "%#{q[:query]}%") unless q[:query].nil?
    conditions
  end

  private

    def owner_presence_in_users
      unless users.empty?
        unless users.collect(&:id).include?(owner_id)
          errors.add(:users, "list does not includes agent owner")
        end
      end
    end

    def add_owner_id
      self.owner_id = users.first.id unless users.empty?
    end

    def clean_agentname
      unless agentname.nil?
        self.agentname = agentname.parameterize(separator: '-')
      end
    end

end
