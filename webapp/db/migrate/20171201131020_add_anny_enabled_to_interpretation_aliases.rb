class AddAnnyEnabledToInterpretationAliases < ActiveRecord::Migration[5.1]
  def change
    add_column :interpretation_aliases, :any_enabled, :boolean, default: false
  end
end
