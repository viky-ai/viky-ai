class AddIntentIdToInterpretationAliases < ActiveRecord::Migration[5.1]
  def change
    add_reference :interpretation_aliases, :intent, foreign_key: { on_delete: :cascade }, type: :uuid
  end
end
