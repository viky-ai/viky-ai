class RenameIntentToInterpretation < ActiveRecord::Migration[6.0]
  def change
    rename_table :intents, :interpretations
    rename_column :interpretations, :intentname, :interpretation_name
    rename_column :formulations, :intent_id, :interpretation_id

    reversible do |direction|
      direction.up do
        FormulationAlias.where(formulation_aliasable_type: 'Intent').update_all(formulation_aliasable_type: 'Interpretation')
        FriendlyId::Slug.where(sluggable_type: 'Intent').update_all(sluggable_type: 'Interpretation')
      end
      direction.down do
        FormulationAlias.where(formulation_aliasable_type: 'Interpretation').update_all(formulation_aliasable_type: 'Intent')
        FriendlyId::Slug.where(sluggable_type: 'Interpretation').update_all(sluggable_type: 'Intent')
      end
    end
  end
end
