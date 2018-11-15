class AddRegExpToInterpretationAliases < ActiveRecord::Migration[5.1]
  def change
    add_column :interpretation_aliases, :reg_exp, :string
  end
end
