class Agent < ApplicationRecord
  extend FriendlyId
  friendly_id :agentname, use: :history, slug_column: 'agentname'

  enum visibility: [:is_private, :is_public]

  require 'rgl/adjacency'

  include AgentImageUploader::Attachment.new(:image)

  has_many :memberships
  has_many :users, through: :memberships
  has_many :intents, dependent: :destroy
  has_many :entities_lists, dependent: :destroy

  has_many :in_arcs,  foreign_key: 'target_id', class_name: 'AgentArc', dependent: :destroy
  has_many :out_arcs, foreign_key: 'source_id', class_name: 'AgentArc', dependent: :destroy

  has_many :predecessors, through: :in_arcs, source: :source
  has_many :successors, through: :out_arcs, source: :target

  validates :name, presence: true
  validates :agentname, uniqueness: { scope: [:owner_id] }, length: { in: 3..25 }, presence: true
  validates :owner_id, presence: true
  validates :api_token, presence: true, uniqueness: true, length: { in: 32..32 }
  validates :color, inclusion: { in: :available_colors }
  validate :owner_presence_in_users

  before_validation :ensure_api_token, on: :create
  before_validation :add_owner_id, on: :create
  before_validation :clean_agentname
  before_destroy :check_collaborators_presence

  after_save do
    if saved_change_to_attribute?(:agentname) || saved_change_to_attribute?(:owner_id)
      Nlp::Package.new(self).push
    end
  end

  after_touch do
    Nlp::Package.new(self).push
  end

  after_destroy do
    Nlp::Package.new(self).destroy
  end

  def self.search(q = {})
    conditions = where('1 = 1')
    conditions = conditions.joins(:memberships)
    conditions = conditions.where('user_id = ? OR visibility = ?', q[:user_id], Agent.visibilities[:is_public])
    unless q[:query].nil?
      conditions = conditions.where(
        'name LIKE ? OR agentname LIKE ?',
        "%#{q[:query]}%",
        "%#{q[:query]}%"
      )
    end
    conditions.distinct
  end

  def available_colors
    [
      'black', 'brown', 'red', 'pink', 'purple', 'deep-purple',
      'indigo', 'blue', 'light-blue', 'cyan', 'teal', 'green', 'light-green',
      'lime', 'yellow', 'amber', 'orange', 'deep-orange'
    ]
  end

  def available_successors(current_user)
    Agent
      .joins(:memberships)
      .where('user_id = ? OR visibility = ?', current_user.id, Agent.visibilities[:is_public])
      .where.not(id: successors.pluck(:id))
      .where.not(id: id)
      .distinct
      .order(name: :asc)
  end

  def transfer_ownership_to(new_owner_id)
    transfer = AgentTransfer.new(self, new_owner_id)
    transfer.proceed
    {
      success: transfer.valid?,
      errors: transfer.errors.flatten
    }
  end

  def owner
    users.includes(:memberships).find_by('memberships.rights' => 'all')
  end

  def collaborators
    users.includes(:memberships).where.not('memberships.rights' => 'all')
  end

  def can_be_destroyed?
    collaborators.count.zero?
  end

  def ensure_api_token
    begin
      self.api_token = SecureRandom.hex
    end while self.class.exists?(api_token: api_token)
  end

  def slug
    "#{User.find(owner_id).username}/#{agentname}"
  end

  def reachable_intents
    result = [] + intents.order(position: :desc, created_at: :desc)
    result + Intent
             .where(visibility: :is_public)
             .where(agent_id: successors.ids)
             .order(position: :desc, created_at: :desc)
  end

  def reachable_entities_lists
    result = [] + entities_lists.order(position: :desc, created_at: :desc)
    result + EntitiesList
             .where(visibility: :is_public)
             .where(agent_id: successors.ids)
             .order(position: :desc, created_at: :desc)
  end

  def update_intents_positions(public_intents, private_intents)
    current_public_intents = Intent.where(agent_id: id, id: public_intents).order(position: :asc)
    current_private_intents = Intent.where(agent_id: id, id: private_intents).order(position: :asc)
    Agent.no_touching do
      update_order(public_intents, Intent.visibilities[:is_public], current_public_intents)
      update_order(private_intents, Intent.visibilities[:is_private], current_private_intents)
    end
    touch
  end

  def update_entities_lists_positions(public_entities_lists, private_entities_lists)
    current_public_entities_lists = EntitiesList.where(agent_id: id, id: public_entities_lists).order(position: :asc)
    current_private_entities_lists = EntitiesList.where(agent_id: id, id: private_entities_lists).order(position: :asc)
    Agent.no_touching do
      update_order(public_entities_lists, EntitiesList.visibilities[:is_public], current_public_entities_lists)
      update_order(private_entities_lists, EntitiesList.visibilities[:is_private], current_private_entities_lists)
    end
    touch
  end


  private
    def update_order(new_ids, visibility, current)
      count = current.count
      current.each do |item|
        new_position = new_ids.find_index(item.id)
        unless new_position.nil?
          item.record_timestamps = false
          item.update_attribute(:position, count - new_position - 1)
          item.record_timestamps = true
          item.update_attribute(:visibility, visibility)
        end
      end
    end

    def check_collaborators_presence
      return if can_be_destroyed?
      errors.add(:base, I18n.t('errors.agent.delete.collaborators_presence'))
      throw(:abort)
    end

    def owner_presence_in_users
      return if memberships.select { |m| m.rights == 'all' }.size == 1
      errors.add(:users, I18n.t('errors.agent.owner_presence_in_users'))
    end

    def add_owner_id
      self.owner_id = memberships.first.user_id unless memberships.empty?
    end

    def clean_agentname
      return if agentname.nil?
      self.agentname = agentname.parameterize(separator: '-')
    end
end
