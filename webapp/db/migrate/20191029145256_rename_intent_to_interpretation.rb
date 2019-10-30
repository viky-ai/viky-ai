class RenameIntentToInterpretation < ActiveRecord::Migration[6.0]
  def change
    rename_table :intents, :interpretations
    rename_column :interpretations, :intentname, :interpretation_name
    rename_column :formulations, :intent_id, :interpretation_id
  end
end
