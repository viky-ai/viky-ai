class InvertQuotaConceptToUsers < ActiveRecord::Migration[5.1]
  def change
    remove_column :users, :ignore_quota
    add_column :users, :quota_enabled, :boolean, default: true
  end
end
