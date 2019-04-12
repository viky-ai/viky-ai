class AddSearchableTermsToEntities < ActiveRecord::Migration[5.1]

  def up
    add_column :entities, :searchable_terms, :text
    Entity.reset_column_information
    Entity.find_each do |entity|
      Rails.logger.silence(Logger::INFO) do
        entity.update_columns(searchable_terms: Entity.extract_searchable_terms(entity.terms))
      end
    end
  end

  def down
    remove_column :entities, :searchable_terms
  end

end
