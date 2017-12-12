class RemoveUniquenessOnInterpretationAliasTable < ActiveRecord::Migration[5.1]
  def change
    remove_index(:interpretation_aliases, [:interpretation_id, :aliasname])
  end
end
