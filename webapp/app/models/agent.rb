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
  validates :api_token, presence: true, uniqueness: true, length: { in: 32..32 }
  validates :color, inclusion: { in: :available_colors }
  validate :owner_presence_in_users

  before_validation :ensure_api_token, on: :create
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

  def ensure_api_token
    begin
      self.api_token = SecureRandom.hex
    end while self.class.exists?(api_token: api_token)
  end


  private

    def owner_presence_in_users
      unless (users.empty? || users.collect(&:id).include?(owner_id))
        errors.add(:users, "list does not includes agent owner")
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
