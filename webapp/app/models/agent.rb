class Agent < ApplicationRecord
  extend FriendlyId
  friendly_id :agentname, use: :history, slug_column: 'agentname'

  enum visibility: [:is_private, :is_public]

  require 'rgl/adjacency'

  include AgentImageUploader::Attachment.new(:image)

  has_one :readme, dependent: :destroy

  has_many :memberships, dependent: :destroy
  has_many :users, through: :memberships
  has_many :favorite_agents, dependent: :destroy
  has_many :fans, through: :favorite_agents, source: :user
  has_many :intents, dependent: :destroy
  has_many :entities_lists, dependent: :destroy
  has_many :bots, dependent: :destroy
  has_many :agent_regression_checks, dependent: :destroy

  has_many :in_arcs,  foreign_key: 'target_id', class_name: 'AgentArc', dependent: :destroy, inverse_of: :target
  has_many :out_arcs, foreign_key: 'source_id', class_name: 'AgentArc', dependent: :destroy, inverse_of: :source

  has_many :predecessors, through: :in_arcs, source: :source
  has_many :successors, through: :out_arcs, source: :target

  serialize :source_agent, JSON

  validates :name, presence: true
  validates :agentname, uniqueness: { scope: [:owner_id] }, length: { in: 3..25 }, presence: true
  validates :owner_id, presence: true
  validates :api_token, presence: true, uniqueness: true, length: { in: 32..32 }
  validates :color, inclusion: { in: :available_colors }
  validate :owner_presence_in_users

  before_validation :ensure_api_token, on: :create
  before_validation :add_owner_id, on: :create
  before_validation :clean_agentname
  before_destroy :check_collaborators_presence, prepend: true

  after_create_commit :nlp_push

  after_update_commit :nlp_push, unless: proc { |record|
    record.previous_changes.size == 2 && record.previous_changes[:nlp_updated_at].present?
  }

  after_destroy do
    Nlp::Package.new(self).destroy
  end

  def self.search(q = {})
    conditions = where('1 = 1')
    conditions = conditions.joins(:memberships)

    case q[:filter_owner]
    when 'owned'
      conditions = conditions.where(owner_id: q[:user_id])
    when 'favorites'
      conditions = conditions.joins(:favorite_agents).where(favorite_agents: { user: q[:user_id] })
    else
      conditions = conditions.where('user_id = ? OR visibility = ?', q[:user_id], Agent.visibilities[:is_public])
    end

    case q[:filter_visibility]
    when 'public'
      conditions = conditions.where(visibility: Agent.visibilities[:is_public])
    when 'private'
      conditions = conditions.where(visibility: Agent.visibilities[:is_private])
    else
      conditions
    end

    if q[:query].present?
      conditions = conditions.where(
        'lower(name) LIKE lower(?) OR lower(agentname) LIKE lower(?) OR lower(description) LIKE lower(?)',
        "%#{q[:query]}%",
        "%#{q[:query]}%",
        "%#{q[:query]}%"
      )
    end

    case q[:sort_by]
    when 'name'
      conditions = conditions.order(name: :asc)
    when 'updated_at'
      conditions = conditions.order(updated_at: :desc)
    else
      conditions
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

  def available_successors(q = {})
    conditions = Agent
                   .joins(:memberships)
                   .where('memberships.user_id = ? OR visibility = ?', q[:user_id], Agent.visibilities[:is_public])
                   .where.not(id: successors.pluck(:id))
    search_available_agents(conditions, q)
      .distinct
  end

  def available_destinations(q = {})
    conditions = User
                   .find(q[:user_id])
                   .agents
                   .where(memberships: { rights: [:all, :edit] })
    search_available_agents(conditions, q)
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

  def collaborator?(user)
    collaborators.where('users.id = ?', user.id).count > 0
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

  def reachable_intents(current_intent)
    result = [] + intents
                    .where.not(id: current_intent)
                    .order(position: :desc, created_at: :desc)
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

  def accessible_bots(user)
    if user.can? :edit, self
      bots
    else
      bots.where(wip_enabled: false)
    end
  end

  def run_tests
    AgentRegressionCheckRunJob.perform_later self
  end

  def json_test_suite(current_user)
    ApplicationController.render(
      template: 'agent_regression_checks/index',
      assigns: { agent: self },
      locals: { user: current_user }
    )
  end

  def any_tests_failed?
    agent_regression_checks.where(state: 'failure').exists?
  end

  def sentence_tested?(sentence, language, now)
    time_parse = (now.kind_of?(String) ? Time.parse(now) : now) unless now.blank?
    agent_regression_checks
      .where('lower(sentence) = lower(?)', sentence.strip)
      .where(language: language)
      .where(now: time_parse)
      .exists?
  end

  def regression_check_for(sentence, language, now)
    time_parse = now.kind_of?(String) ? Time.parse(now) : now
    agent_regression_checks.where('lower(sentence) = lower(?)', sentence.strip)
      .where('lower(sentence) = lower(?)', sentence.strip)
      .where(language: language)
      .where(now: time_parse)
      .first
  end

  def synced_with_nlp?
    return false if nlp_updated_at.nil?
    nlp_updated_at >= updated_at
  end

  private

    def search_available_agents(conditions, q)
      if q[:filter_owner] == 'favorites'
        conditions = conditions
                     .joins(:favorite_agents)
                     .where(favorite_agents: { user: q[:user_id] })
      end
      if q[:query].present?
        conditions = conditions.where(
          'lower(name) LIKE lower(?) OR lower(agentname) LIKE lower(?) OR lower(description) LIKE lower(?)',
          "%#{q[:query]}%",
          "%#{q[:query]}%",
          "%#{q[:query]}%"
        )
      end
      conditions.where.not(id: id)
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

    def nlp_push
      Nlp::Package.new(self).push
    end
end
