class RemoveLocalesFromIntent < ActiveRecord::Migration[5.1]
  def change
    remove_column :intents, :locales, :string
  end
end
