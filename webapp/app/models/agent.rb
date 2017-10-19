class Agent < ApplicationRecord
  extend FriendlyId
  friendly_id :agentname, use: :history, slug_column: 'agentname'

  include AgentImageUploader::Attachment.new(:image)

  has_many :memberships
  has_many :users, through: :memberships

  validates :name, presence: true
  validates :agentname, uniqueness: { scope: [:owner_id] }, length: { in: 3..25 }, presence: true
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

  def transfer_ownership_to(new_owner_username)
    transfert = AgentTransfert.new(self, new_owner_username)
    transfert.proceed
    {
      success: transfert.valid?,
      errors: transfert.errors.flatten
    }
  end

  def owner
    users.includes(:memberships).where('memberships.rights' => 'all').first
  end

  def collaborators
    users.includes(:memberships).where.not('memberships.rights' => 'all')
  end


  private

    def owner_presence_in_users
      unless memberships.select {|m| m.rights == 'all'}.size == 1
        errors.add(:users, "list does not includes agent owner")
      end
    end

    def add_owner_id
      self.owner_id = memberships.first.user_id unless memberships.empty?
    end

    def clean_agentname
      unless agentname.nil?
        self.agentname = agentname.parameterize(separator: '-')
      end
    end

end
