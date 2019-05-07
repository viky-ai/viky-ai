class AddSearchableTermsToEntities < ActiveRecord::Migration[5.1]

  def up
    add_column :entities, :searchable_terms, :text
    puts "---------------------------------------------------------"
    puts "Run command: ./bin/rails entities:update_searchable_terms"
    puts "---------------------------------------------------------"
  end

  def down
    remove_column :entities, :searchable_terms
  end

end
