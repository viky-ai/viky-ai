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

  before_validation :ensure_api_token, on: :create
  before_validation :add_owner_id, on: :create
  before_validation :clean_locales, on: :create
  before_validation :clean_agentname

  validates :name, presence: true
  validates :agentname, uniqueness: { scope: [:owner_id] }, length: { in: 3..25 }, presence: true
  validates :owner_id, presence: true
  validates :api_token, presence: true, uniqueness: true, length: { in: 32..32 }
  validates :color, inclusion: { in: :available_colors }
  validates :locales, presence: true
  validate  :check_locales
  validate  :owner_presence_in_users

  before_save :add_slug
  before_destroy :check_collaborators_presence, prepend: true

  after_create_commit do
    sync_nlp
  end

  after_update_commit do |record|
    if @need_nlp_push || record.previous_changes[:agentname].present?
      agent_regression_checks.update_all(state: 'running')
      notify_tests_suite_ui
      sync_nlp
    end
  end

  after_destroy do
    Nlp::Package.new(self).destroy
  end

  scope :owned_by, ->(user) { where(owner_id: user.id) }

  def self.search(q = {})
    user_reachable = where('1 = 1')
                       .joins(:memberships)
                       .where(memberships: { user_id: q[:user_id] })
    public_reachable = where(visibility: Agent.visibilities[:is_public])

    case q[:filter_owner]
    when 'owned'
      user_reachable = where(owner_id: q[:user_id])
      public_reachable = nil
    when 'favorites'
      user_reachable = user_reachable
                         .joins(:favorite_agents)
                         .where(favorite_agents: { user: q[:user_id] })
      public_reachable = public_reachable
                           .joins(:favorite_agents)
                           .where(favorite_agents: { user: q[:user_id] })
    end

    case q[:filter_visibility]
    when 'public'
      user_reachable = user_reachable.where(visibility: Agent.visibilities[:is_public])
    when 'private'
      user_reachable = user_reachable.where(visibility: Agent.visibilities[:is_private])
      public_reachable = nil
    end

    if q[:query].present?
      user_reachable = user_reachable.joins(:users).where(
        'users.username ILIKE ?
          OR users.name ILIKE ?
          OR agents.name ILIKE ?
          OR agentname ILIKE ?
          OR slug ILIKE ?
          OR description ILIKE ?',
        "%#{q[:query]}%",
        "%#{q[:query]}%",
        "%#{q[:query]}%",
        "%#{q[:query]}%",
        "%#{q[:query]}%",
        "%#{q[:query]}%"
      )
      unless public_reachable.nil?
        public_reachable = public_reachable.joins(:users).where(
          'users.username ILIKE ?
            OR users.name ILIKE ?
            OR agents.name ILIKE ?
            OR agentname ILIKE ?
            OR slug ILIKE ?
            OR description ILIKE ?',
          "%#{q[:query]}%",
          "%#{q[:query]}%",
          "%#{q[:query]}%",
          "%#{q[:query]}%",
          "%#{q[:query]}%",
          "%#{q[:query]}%"
        )
      end
    end

    case q[:selected]
    when 'true'
      user_reachable = user_reachable.where(id: q[:selected_ids])
      public_reachable = public_reachable.where(id: q[:selected_ids]) unless public_reachable.nil?
    when 'false'
      user_reachable = user_reachable.where.not(id: q[:selected_ids])
      public_reachable = public_reachable.where.not(id: q[:selected_ids]) unless public_reachable.nil?
    end

    select_request = !public_reachable.nil? ? "#{user_reachable.to_sql} UNION #{public_reachable.to_sql}" : user_reachable.to_sql
    sort_request = Agent.from(Arel.sql "(#{select_request}) AS agents")
    case q[:sort_by]
    when 'name'
      sort_request = sort_request.order(name: :asc)
    when 'updated_at'
      sort_request = sort_request.order(updated_at: :desc, name: :asc)
    when 'popularity'
      popularity_sort = sort_request
                          .select(Arel.sql "agents.id, COUNT(agent_arcs.target_id) AS count_agents")
                          .left_joins(:in_arcs)
                          .group(:id)
      sort_request = Agent.from(Arel.sql "agents, (#{popularity_sort.to_sql}) AS ags").where(Arel.sql 'agents.id = ags.id').order('ags.count_agents desc, agents.name asc')
    end

    sort_request
  end

  def available_colors
    [
      'black', 'brown', 'red', 'pink', 'purple', 'deep-purple',
      'indigo', 'blue', 'light-blue', 'cyan', 'teal', 'green', 'light-green',
      'lime', 'yellow', 'amber', 'orange', 'deep-orange'
    ]
  end

  def ordered_locales
    Locales::ALL.select { |l| locales.include?(l) }
  end

  def ordered_and_used_locales
    entities_locales = Entity.select("distinct value->'locale' as language")
          .where(entities_list_id: entities_lists.pluck(:id))
          .from("entities, jsonb_array_elements(entities.terms)")
          .collect{|e| e['language']}

    interpretations_locales = Interpretation.select(:locale)
      .where(intent_id: intents.pluck(:id))
      .distinct
      .collect{|i| i.locale}.flatten.uniq

    all_used_locales = (entities_locales + interpretations_locales).flatten.uniq
    Locales::ALL.select { |l| all_used_locales.include?(l) }
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
    users.find_by('memberships.rights' => 'all')
  end

  def collaborators
    users.where.not('memberships.rights' => 'all')
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

  def expressions_count
    entities_count = entities_lists.sum(:entities_count)
    interpretations_count = Interpretation.joins(intent: :agent).where('agents.id = ?', id).count
    entities_count + interpretations_count
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

  def run_regression_checks
    if agent_regression_checks.exists?
      AgentRegressionCheckRunJob.perform_later self
    else
      notify_tests_suite_ui
    end
  end

  def regression_checks_to_json
    ApplicationController.render(
      template: 'agent_regression_checks/index',
      assigns: { agent: self }
    )
  end

  def regression_checks_global_state
    return 'running' if agent_regression_checks.where(state: 'running').exists?
    return 'error'   if agent_regression_checks.where(state: 'error').exists?
    return 'failure' if agent_regression_checks.where(state: 'failure').exists?
    return 'success' if agent_regression_checks.where(state: 'success').count == agent_regression_checks.count
    'unknown'
  end

  def find_regression_check_with(sentence, language, spellchecking, now)
    time_parse = (now.kind_of?(String) ? Time.parse(now) : now) unless now.blank?
    agent_regression_checks
      .where('lower(sentence) = lower(?)', sentence.strip)
      .where(language: language)
      .where(spellchecking: spellchecking)
      .where(now: time_parse)
      .first
  end

  def need_nlp_sync
    @need_nlp_push = true
  end

  def synced_with_nlp?
    return false if nlp_updated_at.nil?
    nlp_updated_at >= updated_at
  end

  def update_locales
    used_locales = ordered_and_used_locales
    agent_locales = (locales + used_locales).uniq
    if agent_locales != locales
      update_columns(locales: agent_locales)
    end
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

    def add_slug
      if agentname_changed?
        self.slug = "#{User.find(owner_id).username}/#{agentname}"
      end
    end

    def clean_agentname
      return if agentname.nil?
      self.agentname = agentname.parameterize(separator: '-')
    end

    def clean_locales
      if locales.blank?
        self.locales = [Locales::ANY, 'en', 'fr']
      else
        self.locales = locales.uniq
      end
    end

    def check_locales
      return if locales.blank?
      locales.each do |locale|
        unless Locales::ALL.include? locale
          errors.add(:locales, I18n.t('errors.agent.unknown_locale', current_locale: locale))
        end
      end
    end

    def notify_tests_suite_ui
      ActionCable.server.broadcast(
        "agent_console_channel_#{self.id}",
        agent_id: self.id,
        timestamp: Time.now.to_f * 1000,
        payload: JSON.parse(regression_checks_to_json)
      )
    end

    def sync_nlp
      Nlp::Package.new(self).push
      @need_nlp_push = false
    end
end
