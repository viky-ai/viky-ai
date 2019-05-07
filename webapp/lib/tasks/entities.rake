namespace :entities do

  desc "Update all entities searchable_terms"
  task :update_searchable_terms => [:environment] do |t, args|
    Entity.find_each do |entity|
      Rails.logger.silence(Logger::INFO) do
        entity.update_columns(searchable_terms: Entity.extract_searchable_terms(entity.terms))
      end
    end
  end

end
