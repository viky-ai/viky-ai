class AddColumnMultipleToInterpretationAliasesTable < ActiveRecord::Migration[5.1]
  def change
    add_column :interpretation_aliases, :is_list, :boolean, default: false
  end
end
