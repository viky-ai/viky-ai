class UseJsonb < ActiveRecord::Migration[5.1]
  def up
    change_column :agents, :source_agent, :jsonb, using: 'source_agent::text::jsonb'
    change_column :agent_regression_checks, :expected, :jsonb, using: 'expected::text::jsonb'
    change_column :agent_regression_checks, :got, :jsonb, using: 'got::text::jsonb'
    change_column :chat_statements, :content, :jsonb, using: 'content::text::jsonb'

    change_column :entities, :terms, :jsonb, using: 'terms::text::jsonb'
    add_index :entities, :terms, using: :gin

    change_column_default :users, :ui_state, nil
    change_column :users, :ui_state, :jsonb, using: 'ui_state::text::jsonb'
    change_column_default :users, :ui_state, {}

    Entity.reset_column_information
    Agent.reset_column_information
    Agent.find_each do |agent|
      agent.update_columns(locales: agent.ordered_and_used_locales)
    end
  end

  def down
  end
end
