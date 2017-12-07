class Agent < ApplicationRecord
  extend FriendlyId
  friendly_id :agentname, use: :history, slug_column: 'agentname'

  require 'rgl/adjacency'

  include AgentImageUploader::Attachment.new(:image)

  has_many :memberships
  has_many :users, through: :memberships
  has_many :intents, dependent: :destroy

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

  after_destroy do
    Nlp::Package.new(self).destroy
  end

  def self.search(q = {})
    conditions = where("1 = 1").joins(:memberships).where("user_id = ?", q[:user_id])
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

  def available_successors(current_user)
    Agent.joins(:memberships).
      where("user_id = ?", current_user.id).
      where.not(id: self.successors.select(:id).collect(&:id)).
      where.not(id: self.id).
      order(name: :asc)
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
    collaborators.count == 0
  end

  def ensure_api_token
    begin
      self.api_token = SecureRandom.hex
    end while self.class.exists?(api_token: api_token)
  end

  def slug
    "#{owner.slug}/#{agentname}"
  end

  def to_graph(&block)
    graph = RGL::DirectedAdjacencyGraph.new
    node = block_given? ? yield(self) : self
    graph.add_vertex(node)
    list = block_given? ? list_out_arcs(&block) : list_out_arcs
    list.each do |(src, target)|
      graph.add_edge(src, target)
    end
    graph
  end

  def list_out_arcs(&block)
    arcs_list = []
    if block_given?
      arcs_list += successors.reload.map { |successor| [yield(self), yield(successor)] }
    else
      arcs_list += successors.reload.map { |successor| [self, successor] }
    end
    successors.each do |successor|
      arcs_list += block_given? ? successor.list_out_arcs(&block) : successor.list_out_arcs
    end
    arcs_list
  end

  def to_predecessors_graph(&block)
    graph = RGL::DirectedAdjacencyGraph.new
    node = block_given? ? yield(self) : self
    graph.add_vertex(node)
    list = block_given? ? list_in_arcs(&block) : list_in_arcs
    list.each do |(src, target)|
      graph.add_edge(src, target)
    end
    graph
  end

  def list_in_arcs(&block)
    arcs_list = []
    if block_given?
      arcs_list += predecessors.reload.map { |predecessor| [yield(predecessor), yield(self)] }
    else
      arcs_list += predecessors.reload.map { |predecessor| [self, predecessor] }
    end
    predecessors.each do |predecessor|
      arcs_list += block_given? ? predecessor.list_in_arcs(&block) : predecessor.list_in_arcs
    end
    arcs_list
  end


  private

    def check_collaborators_presence
      unless can_be_destroyed?
        errors.add(:base, I18n.t('errors.agent.delete.collaborators_presence'))
        throw(:abort)
      end
    end

    def owner_presence_in_users
      unless memberships.select {|m| m.rights == 'all'}.size == 1
        errors.add(:users, I18n.t('errors.agent.owner_presence_in_users'))
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
