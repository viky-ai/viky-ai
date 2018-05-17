class ResetCache < ActiveRecord::Migration[5.1]
  def change
    Rails.cache.clear
  end
end
