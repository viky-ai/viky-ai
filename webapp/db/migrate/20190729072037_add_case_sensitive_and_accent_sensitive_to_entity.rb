class AddCaseSensitiveAndAccentSensitiveToEntity < ActiveRecord::Migration[5.1]
  def change
    add_column :entities, :case_sensitive, :boolean, default: false
    add_column :entities, :accent_sensitive, :boolean, default: false
  end
end
