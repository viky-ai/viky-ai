class AddIntentVisibilityColumn < ActiveRecord::Migration[5.1]
  def change
    add_column :intents, :visibility, :integer, default: 0
  end
end
