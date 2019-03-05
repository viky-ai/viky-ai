class AddLocalesToEntity < ActiveRecord::Migration[5.1]
  def up
    add_column :entities, :locales, :jsonb
    Entity.reset_column_information
    Entity.find_each do |entity|
      locales = entity.terms.collect{ |t| t["locale"] }.uniq
      entity.update_columns(locales: locales)
    end
    Agent.find_each do |agent|
      used_locales = agent.ordered_and_used_locales
      agent.update_columns(locales: used_locales)
    end
  end

  def down
    remove_column :entities, :locales
  end
end
