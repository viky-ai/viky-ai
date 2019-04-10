class AddMetaToEntitiesImports < ActiveRecord::Migration[5.1]
  def change
    add_column :entities_imports, :status, :integer, default: 0
    add_column :entities_imports, :duration, :integer, default: 0
    add_column :entities_imports, :filesize, :integer, default: 0
    add_column :entities_imports, :user_id, :uuid
  end
end
