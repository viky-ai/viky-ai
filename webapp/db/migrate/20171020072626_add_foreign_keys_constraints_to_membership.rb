class AddForeignKeysConstraintsToMembership < ActiveRecord::Migration[5.1]
  def change
    add_foreign_key :memberships, :users, on_delete: :cascade
    add_foreign_key :memberships, :agents, on_delete: :cascade
  end
end
