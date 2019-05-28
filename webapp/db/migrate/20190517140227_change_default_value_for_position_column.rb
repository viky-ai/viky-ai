class ChangeDefaultValueForPositionColumn < ActiveRecord::Migration[5.1]
  def change
    change_column_default(:intents, :position, -1)
    change_column_default(:interpretations, :position, -1)
    change_column_default(:entities_lists, :position, -1)
    change_column_default(:entities, :position, -1)
    change_column_default(:agent_regression_checks, :position, -1)
  end
end
