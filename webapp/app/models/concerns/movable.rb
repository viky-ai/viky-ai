module Movable
  extend ActiveSupport::Concern

  def available_destinations(q = {})
    current_user = User.find(q[:user_id])
    conditions = current_user.agents
      .where(memberships: { rights: [:all, :edit] })
      .where.not(id: agent.id)
    if q[:filter_owner] == 'favorites'
      conditions = conditions
                     .joins(:favorite_agents)
                     .where(favorite_agents: { user: q[:user_id]})
    end
    if q[:query].present?
      conditions = conditions.where(
        'lower(name) LIKE lower(?) OR lower(agentname) LIKE lower(?) OR lower(description) LIKE lower(?)',
        "%#{q[:query]}%",
        "%#{q[:query]}%",
        "%#{q[:query]}%"
      )
    end
    conditions
  end

  def change_agent(current_user, agent_destination)
    interpretation_aliasable_name = ActiveModel::Naming.plural(self.class)
    if agent_destination.nil?
      errors.add(:agent, I18n.t('errors.' + interpretation_aliasable_name + '.unknown_agent'))
      return false
    end
    unless current_user.can?(:edit, agent_destination)
      errors.add(:agent, I18n.t('errors.' + interpretation_aliasable_name + '.not_editable_agent'))
      return false
    end
    ActiveRecord::Base.transaction do
      self.agent = agent_destination
      interpretation_aliasable = agent_destination.send(interpretation_aliasable_name)
      self.position = interpretation_aliasable.present? ? interpretation_aliasable.maximum(:position) + 1 : 0
      save
    end
  end
end
