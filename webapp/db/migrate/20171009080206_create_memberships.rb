class CreateMemberships < ActiveRecord::Migration[5.1]
  def change
    create_table :memberships do |t|
      t.uuid :user_id
      t.uuid :agent_id

      t.timestamps
    end
  end
end
