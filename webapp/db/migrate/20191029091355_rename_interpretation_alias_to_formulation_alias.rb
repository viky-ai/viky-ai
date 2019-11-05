class RenameInterpretationAliasToFormulationAlias < ActiveRecord::Migration[6.0]
  def change
    rename_table :interpretation_aliases, :formulation_aliases
    rename_column :formulation_aliases, :interpretation_aliasable_id, :formulation_aliasable_id
    rename_column :formulation_aliases, :interpretation_aliasable_type, :formulation_aliasable_type
  end
end
