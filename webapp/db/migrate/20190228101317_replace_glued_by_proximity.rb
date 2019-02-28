class ReplaceGluedByProximity < ActiveRecord::Migration[5.1]
  def change
    add_column :interpretations, :proximity, :integer, default: 20

    Interpretation.where(glued: true).in_batches do |interpretations|
      interpretations.update_all(proximity: 'glued')
    end

    Interpretation.where(glued: false).in_batches do |interpretations|
      interpretations.update_all(proximity: 'close')
    end

    # TODO: uncomment
    # remove_column :interpretations, :glued, :boolean
  end
end
