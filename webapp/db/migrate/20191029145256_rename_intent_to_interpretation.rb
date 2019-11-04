class RenameIntentToInterpretation < ActiveRecord::Migration[6.0]
  def change
    rename_table :intents, :interpretations
    rename_column :interpretations, :intentname, :interpretation_name
    rename_column :formulations, :intent_id, :interpretation_id

    reversible do |direction|
      direction.up do
        FormulationAlias.where(formulation_aliasable_type: 'Intent').update_all(formulation_aliasable_type: 'Interpretation')
        FriendlyId::Slug.where(sluggable_type: 'Intent').update_all(sluggable_type: 'Interpretation')
        execute <<-SQL
          UPDATE agent_regression_checks AS arc
          SET expected = jsonb_set(expected, '{root_type}'::text[], '"interpretation"'::jsonb, false)
          WHERE arc.expected::jsonb @> '{"root_type": "intent"}'::jsonb;
        SQL
        execute <<-SQL
          UPDATE agent_regression_checks AS arc
          SET got = jsonb_set(got, '{root_type}'::text[], '"interpretation"'::jsonb, false)
          WHERE arc.got::jsonb @> '{"root_type": "intent"}'::jsonb;
        SQL
      end
      direction.down do
        FormulationAlias.where(formulation_aliasable_type: 'Interpretation').update_all(formulation_aliasable_type: 'Intent')
        FriendlyId::Slug.where(sluggable_type: 'Interpretation').update_all(sluggable_type: 'Intent')
        execute <<-SQL
          UPDATE agent_regression_checks AS arc
          SET expected = jsonb_set(expected, '{root_type}'::text[], '"intent"'::jsonb, false)
          WHERE arc.expected::jsonb @> '{"root_type": "interpretation"}'::jsonb;
        SQL
        execute <<-SQL
          UPDATE agent_regression_checks AS arc
          SET got = jsonb_set(got, '{root_type}'::text[], '"intent"'::jsonb, false)
          WHERE arc.got::jsonb @> '{"root_type": "interpretation"}'::jsonb;
        SQL
      end
    end
  end
end
