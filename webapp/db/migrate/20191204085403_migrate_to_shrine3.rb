class MigrateToShrine3 < ActiveRecord::Migration[6.0]

  def up
    Agent.find_each do |agent|
      agent.image_attacher.write
      agent.image_attacher.atomic_persist
    end

    User.find_each do |user|
      user.image_attacher.write
      user.image_attacher.atomic_persist
    end

    EntitiesImport.find_each do |import|
      import.file_attacher.write
      import.file_attacher.atomic_persist
    end
  end

  def down
  end

end
