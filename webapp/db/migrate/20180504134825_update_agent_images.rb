class UpdateAgentImages < ActiveRecord::Migration[5.1]

  def up
    Agent.find_each do |agent|
      if agent.image_attacher.stored?
        agent.update(image: agent.image[:original])
      end
    end
  end

  def down
  end

end
