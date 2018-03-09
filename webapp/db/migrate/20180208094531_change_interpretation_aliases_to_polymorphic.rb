class ChangeInterpretationAliasesToPolymorphic < ActiveRecord::Migration[5.1]
  def change
    remove_foreign_key :interpretation_aliases, :intents
    remove_index :interpretation_aliases, :intent_id

    add_column :interpretation_aliases, :interpretation_aliasable_type, :string
    rename_column :interpretation_aliases, :intent_id, :interpretation_aliasable_id

    ActiveRecord::Base.record_timestamps = false
    InterpretationAlias.type_intent.update_all(interpretation_aliasable_type: 'Intent')
    ActiveRecord::Base.record_timestamps = true

    add_index :interpretation_aliases, [:interpretation_aliasable_type, :interpretation_aliasable_id], name: 'index_ialiases_on_ialiasable_type_and_ialiasable_id'
  end
end
