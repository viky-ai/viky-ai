class AddUniquenessConstraintToInterpretationAliasesTable < ActiveRecord::Migration[5.1]
  def change
    add_index(:interpretation_aliases, [:interpretation_id, :aliasname], unique: true)
  end
end
