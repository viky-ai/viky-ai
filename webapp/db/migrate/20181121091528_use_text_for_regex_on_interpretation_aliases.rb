class UseTextForRegexOnInterpretationAliases < ActiveRecord::Migration[5.1]
  def up
    change_column :interpretation_aliases, :reg_exp, :text
  end
  def down
    change_column :interpretation_aliases, :reg_exp, :string
  end
end
