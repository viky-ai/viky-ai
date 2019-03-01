class ReplaceGluedByProximity < ActiveRecord::Migration[5.1]
  def up
    add_column :interpretations, :proximity, :integer, default: 2

    Interpretation.where(glued: true).in_batches do |interpretations|
      interpretations.update_all(proximity: 'glued')
    end

    Interpretation.where(glued: false).in_batches do |interpretations|
      interpretations.update_all(proximity: 'close')
    end

    remove_column :interpretations, :glued, :boolean
  end

  def down
    add_column :interpretations, :glued, :boolean, default: false

    Interpretation.where(proximity: 'glued').in_batches do |interpretations|
      interpretations.update_all(glued: true)
    end

    Interpretation.where.not(proximity: 'glued').in_batches do |interpretations|
      interpretations.update_all(glued: false)
    end

    remove_column :interpretations, :proximity
  end
end
