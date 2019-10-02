class AddIgnoreQuotaToUsers < ActiveRecord::Migration[5.1]
  def change
    add_column :users, :ignore_quota, :boolean, :default => false
  end
end
