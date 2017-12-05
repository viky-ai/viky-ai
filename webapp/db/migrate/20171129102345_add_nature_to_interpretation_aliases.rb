class AddNatureToInterpretationAliases < ActiveRecord::Migration[5.1]
  def change
    add_column :interpretation_aliases, :nature, :integer, default: 0
  end
end
