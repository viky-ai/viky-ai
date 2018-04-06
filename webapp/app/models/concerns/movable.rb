module Movable
  extend ActiveSupport::Concern

  def move_to_agent(agent_destination)
    interpretation_aliasable_name = ActiveModel::Naming.plural(self.class)
    if agent_destination.nil?
      errors.add(:agent, I18n.t('errors.' + interpretation_aliasable_name + '.unknown_agent'))
      return false
    end
    self.agent = agent_destination
    interpretation_aliasable = agent_destination.send(interpretation_aliasable_name)
    self.position = interpretation_aliasable.present? ? interpretation_aliasable.maximum(:position) + 1 : 0
    save
  end
end
