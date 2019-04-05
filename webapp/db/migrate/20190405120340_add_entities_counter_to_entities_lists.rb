# frozen_string_literal: true

class AddEntitiesCounterToEntitiesLists < ActiveRecord::Migration[5.1]
  def change
    add_column :entities_lists, :entities_count, :integer, default: 0, null: false

    reversible do |dir|
      dir.up { populate_counter }
    end
  end

  def populate_counter
    execute <<-SQL.squish
      UPDATE entities_lists
        SET entities_count = (SELECT count(1)
                              FROM entities
                              WHERE entities.entities_list_id = entities_lists.id)
    SQL
  end
end
