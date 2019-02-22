class CreateAgentRegressionChecks < ActiveRecord::Migration[5.1]
  def change
    create_table :agent_regression_checks, id: :uuid do |t|
      t.string :sentence
      t.string :language
      t.datetime :now
      t.references :agent, foreign_key: { on_delete: :cascade }, type: :uuid
      t.text :expected
      t.text :got
      t.integer :state, default: 0
      t.integer :position, default: 0

      t.timestamps
    end
  end
end
