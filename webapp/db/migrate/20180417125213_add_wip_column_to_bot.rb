class AddWipColumnToBot < ActiveRecord::Migration[5.1]
  def change
    add_column :bots, :wip_enabled, :boolean, default: true
  end
end
