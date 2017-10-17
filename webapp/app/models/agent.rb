class Agent < ApplicationRecord
  extend FriendlyId
  friendly_id :agentname, use: :history, slug_column: 'agentname'

  include AgentImageUploader::Attachment.new(:image)

  has_many :memberships
  has_many :users, through: :memberships

  validates :name, presence: true
  validates :agentname, uniqueness: { scope: [:owner_id] }, length: { in: 3..25 }, presence: true
  validates :users, presence: true
  validates :owner_id, presence: true
  validates :color, inclusion: { in: :available_colors }
  validate :owner_presence_in_users

  before_validation :add_owner_id, on: :create
  before_validation :clean_agentname

  def self.search(q = {})
    conditions = where("1 = 1")
    conditions = joins(:memberships).where("user_id = ?", q[:user_id])
    conditions = conditions.where("name LIKE ? OR agentname LIKE ?", "%#{q[:query]}%", "%#{q[:query]}%") unless q[:query].nil?
    conditions
  end

  def available_colors
    [
      'black', 'brown', 'red', 'pink', 'purple', 'deep-purple',
      'indigo', 'blue', 'light-blue', 'cyan', 'teal', 'green', 'light-green',
      'lime', 'yellow', 'amber', 'orange', 'deep-orange'
    ]
  end

  def transfer_ownership_to(new_owner)
    errors = []
    ActiveRecord::Base.transaction do
      has_agent_with_this_agentname = new_owner.agents.where(agentname: self.agentname).count != 0
      if has_agent_with_this_agentname
        errors << I18n.t('errors.agent.transfer_ownership.duplicate_agentname')
        raise ActiveRecord::Rollback
      end

      self.memberships.where(user_id: owner_id).first.destroy
      new_membership = Membership.new(agent_id: id, user_id: new_owner.id)
      if new_membership.save
        self.owner_id = new_owner.id
        unless self.save
          errors << self.errors.full_messages
          raise ActiveRecord::Rollback
        end
      else
        errors << new_membership.errors.full_messages
        raise ActiveRecord::Rollback
      end
    end
    return {
      success: errors.empty?,
      errors: errors.flatten
    }
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
