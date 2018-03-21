class AddColumnUserStateToUser < ActiveRecord::Migration[5.1]
  def change
    add_column :users, :ui_state, :string, default: {}.to_json
  end
end
