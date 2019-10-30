module Movable
  extend ActiveSupport::Concern

  def move_to_agent(agent_destination)
    aliasable_name = ActiveModel::Naming.plural(self.class)

    if agent_destination.nil?
      errors.add(:agent, I18n.t('errors.' + aliasable_name + '.move_to_unknown_agent'))
      return false
    end

    if Feature.quota_enabled? && agent_destination.owner.quota_enabled
      if aliasable_name == "interpretations"
        expressions_count_addition = self.formulations.count
      end
      if aliasable_name == "entities_lists"
        expressions_count_addition = self.entities_count
      end
      if agent_destination.owner.expressions_count + expressions_count_addition > Quota.expressions_limit
        errors.add(
          :agent,
          I18n.t('errors.' + aliasable_name + '.move_to_agent_without_enough_quota')
        )
        return false
      end
    end

    self.agent = agent_destination
    formulation_aliasable = agent_destination.send(aliasable_name)
    self.position = formulation_aliasable.present? ? formulation_aliasable.maximum(:position) + 1 : 0
    save
  end
end
